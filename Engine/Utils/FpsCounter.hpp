#pragma once

/**
 * @brief Frame rate and frame time, averaged over fixed periods
 *
 * Averaging keeps the numbers readable: per-frame values flicker.
 */
class FpsCounter {
private:
  double period;

  // Current period
  double elapsed = 0;
  int frames = 0;

  // Last finished period
  double fps = 0;
  double frameTime = 0;

public:
  /**
   * @brief Construct a new FpsCounter object
   *
   * @param period Averaging period in seconds
   */
  explicit FpsCounter(double period = 0.5): period(period) {}

  /**
   * @brief Count a frame
   *
   * @param dt Real time since the previous frame in seconds, not scaled
   *        by the game speed
   */
  void tick(double dt) {
    elapsed += dt;
    ++frames;

    if (elapsed >= period) {
      fps = frames / elapsed;
      frameTime = elapsed / frames * 1000.0;

      elapsed = 0;
      frames = 0;
    }
  }

  /**
   * @brief Frames per second over the last period, 0 before the first
   */
  double getFps() const {
    return fps;
  }

  /**
   * @brief Average frame time over the last period in milliseconds
   */
  double getFrameTime() const {
    return frameTime;
  }
};
