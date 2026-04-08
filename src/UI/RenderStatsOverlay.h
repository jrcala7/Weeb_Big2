#pragma once

#include <cstdint>
#include <chrono>
#include <deque>

class Model3D;

/// @brief Displays real-time rendering statistics in an ImGui overlay.
///        Shows FPS, render time, vertex count, and triangle count.
class RenderStatsOverlay final {
public:
    RenderStatsOverlay() = default;
    ~RenderStatsOverlay() = default;

    RenderStatsOverlay(const RenderStatsOverlay&) = delete;
    RenderStatsOverlay& operator=(const RenderStatsOverlay&) = delete;
    RenderStatsOverlay(RenderStatsOverlay&&) = delete;
    RenderStatsOverlay& operator=(RenderStatsOverlay&&) = delete;

    /// Begin frame timing. Call at the start of each render frame.
    void BeginFrame();

    /// End frame timing. Call at the end of each render frame.
    /// @param dt  Delta time in seconds since the last frame.
    void EndFrame(float dt);

    /// Update mesh statistics from the given model.
    /// @param model  The model whose vertex/triangle counts will be tracked.
    void UpdateModelStats(const Model3D& model);

    /// Draw the stats overlay UI using ImGui.
    /// Should be called within an ImGui frame (BIG2_SCOPE_VAR(big2::ImGuiFrameScoped)).
    void Draw();

private:
    // Frame timing
    std::chrono::high_resolution_clock::time_point frame_start_;
    float last_frame_time_ = 0.0f;

    // FPS calculation
    static constexpr int FRAME_HISTORY_SIZE = 60;
    float frame_times_[FRAME_HISTORY_SIZE] = {};
    int frame_index_ = 0;
    float average_frame_time_ = 0.0f;

    // 10-second FPS accumulation
    static constexpr float LONG_TERM_WINDOW = 10.0f;
    std::deque<float> frame_times_long_term_;
    float long_term_accumulation_ = 0.0f;

    // Mesh statistics
    std::uint32_t total_vertex_count_ = 0;
    std::uint32_t total_triangle_count_ = 0;

    // Memory tracking
    float current_memory_mb_ = 0.0f;
    float gpu_memory_used_mb_ = 0.0f;
    float gpu_memory_available_mb_ = 0.0f;

    /// Calculate average FPS from frame history.
    [[nodiscard]] float CalculateAverageFPS() const;

    /// Calculate average FPS over 10 seconds as an integer.
    [[nodiscard]] std::uint32_t CalculateTenSecondAverageFPS() const;

    /// Get current memory usage in MB.
    [[nodiscard]] float GetMemoryUsageMB() const;

    /// Update GPU memory statistics from BGFX.
    void UpdateGPUMemoryStats();
};

/// @brief Convenience function to draw the render stats overlay.
///        Should be called within an ImGui frame.
void DrawRenderStatsOverlay(RenderStatsOverlay& stats);
