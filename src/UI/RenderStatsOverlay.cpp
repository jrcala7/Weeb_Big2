#include "RenderStatsOverlay.h"

#include "Data/Model3D.h"

#if BIG2_IMGUI_ENABLED
#include <big2/imgui/imgui.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

#include <bgfx/bgfx.h>

void RenderStatsOverlay::BeginFrame() {
    frame_start_ = std::chrono::high_resolution_clock::now();
}

void RenderStatsOverlay::EndFrame(float dt) {
    auto frame_end = std::chrono::high_resolution_clock::now();
    auto frame_duration = std::chrono::duration<float>(frame_end - frame_start_).count();
    last_frame_time_ = frame_duration;

    // Update frame time history for FPS calculation
    frame_times_[frame_index_] = dt;
    frame_index_ = (frame_index_ + 1) % FRAME_HISTORY_SIZE;

    // Calculate average frame time
    float sum = 0.0f;
    for (int i = 0; i < FRAME_HISTORY_SIZE; ++i) {
        sum += frame_times_[i];
    }
    average_frame_time_ = sum / FRAME_HISTORY_SIZE;

    // Update 10-second FPS accumulation
    frame_times_long_term_.push_back(dt);
    long_term_accumulation_ += dt;

    // Remove frames older than 10 seconds
    while (!frame_times_long_term_.empty() && long_term_accumulation_ > LONG_TERM_WINDOW) {
        long_term_accumulation_ -= frame_times_long_term_.front();
        frame_times_long_term_.pop_front();
    }

    // Update memory usage
    current_memory_mb_ = GetMemoryUsageMB();
    UpdateGPUMemoryStats();
}

void RenderStatsOverlay::UpdateModelStats(const Model3D& model) {
    total_vertex_count_ = 0;
    total_triangle_count_ = 0;

    const auto& meshes = model.GetMeshes();
    for (const auto& mesh : meshes) {
        total_vertex_count_ += static_cast<std::uint32_t>(mesh.vertices.size());
        // Each group of 3 indices forms a triangle
        total_triangle_count_ += static_cast<std::uint32_t>(mesh.indices.size()) / 3;
    }
}

float RenderStatsOverlay::CalculateAverageFPS() const {
    if (average_frame_time_ <= 0.0f) {
        return 0.0f;
    }
    return 1.0f / average_frame_time_;
}

std::uint32_t RenderStatsOverlay::CalculateTenSecondAverageFPS() const {
    if (frame_times_long_term_.empty() || long_term_accumulation_ <= 0.0f) {
        return 0;
    }
    std::uint32_t frame_count = static_cast<std::uint32_t>(frame_times_long_term_.size());
    float fps = frame_count / long_term_accumulation_;
    return static_cast<std::uint32_t>(fps + 0.5f);  // Round to nearest integer
}

float RenderStatsOverlay::GetMemoryUsageMB() const {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    HANDLE process_handle = GetCurrentProcess();

    if (GetProcessMemoryInfo(process_handle, &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024.0f * 1024.0f);  // Convert bytes to MB
    }
    return 0.0f;
#else
    // Platform-independent fallback: return 0 on non-Windows
    return 0.0f;
#endif
}

void RenderStatsOverlay::UpdateGPUMemoryStats() {
    const bgfx::Stats* stats = bgfx::getStats();
    if (stats) {
        // BGFX reports GPU memory in bytes
        gpu_memory_used_mb_ = stats->gpuMemoryUsed / (1024.0f * 1024.0f);
        // gpuMemoryMax provides the total GPU memory available
        gpu_memory_available_mb_ = stats->gpuMemoryMax / (1024.0f * 1024.0f);
    }
}

void RenderStatsOverlay::Draw() {
#if BIG2_IMGUI_ENABLED
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Render Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        float fps = CalculateAverageFPS();
        std::uint32_t ten_sec_fps = CalculateTenSecondAverageFPS();

        // FPS display
        ImGui::Text("FPS: %.1f", fps);
        ImGui::SameLine();
        ImGui::Text("(%.2f ms)", average_frame_time_ * 1000.0f);

        // 10-second average FPS
        ImGui::Text("10s Avg FPS: %u", ten_sec_fps);

        // Frame time
        ImGui::Text("Frame Time: %.3f ms", last_frame_time_ * 1000.0f);

        ImGui::Separator();

        // Vertex and triangle counts
        ImGui::Text("Vertices: %u", total_vertex_count_);
        ImGui::Text("Triangles: %u", total_triangle_count_);

        ImGui::Separator();

        // CPU Memory usage
        ImGui::Text("CPU Memory: %.1f MB", current_memory_mb_);

        // GPU Memory usage
        if (gpu_memory_available_mb_ > 0.0f) {
            ImGui::Text("GPU Memory: %.1f / %.1f MB", gpu_memory_used_mb_, gpu_memory_available_mb_);
        } else {
            ImGui::Text("GPU Memory: %.1f MB", gpu_memory_used_mb_);
        }

        ImGui::End();
    }
#endif
}

void DrawRenderStatsOverlay(RenderStatsOverlay& stats) {
    stats.Draw();
}
