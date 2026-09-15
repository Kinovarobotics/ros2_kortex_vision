
#include <ros_kortex_vision/vision.h>

#include <rclcpp/rclcpp.hpp>

#include <csignal>

std::shared_ptr<ros_kortex_vision::Vision> node;

int main(int argc, char** argv)
{
  // rclcpp installs its own SIGINT/SIGTERM handling. The previous custom handler called
  // quit(), rclcpp::shutdown() and exit() from inside the signal handler while run() was
  // still using GStreamer, which segfaulted (-11), aborted (-6) or hung on Ctrl+C.
  rclcpp::init(argc, argv);

  // Closing the terminal sends SIGHUP, which rclcpp does not handle: the default action killed
  // the process without closing the RTSP session and the camera refused new streams for >12 s.
  // raise() is async-signal-safe; it routes SIGHUP through rclcpp's graceful SIGINT path.
  std::signal(SIGHUP, [](int) { std::raise(SIGINT); });

  auto opt = rclcpp::NodeOptions().use_intra_process_comms(true);
  node = std::make_shared<ros_kortex_vision::Vision>(opt);

  // Runs in rclcpp's signal-handling thread, not in the signal handler: unblocks a pending
  // gst_app_sink_pull_sample() so run() can leave its loop and tear the stream down.
  rclcpp::on_shutdown([]() {
    if (node)
    {
      node->quit();
    }
  });

  node->run();
  node.reset();
  rclcpp::shutdown();

  return 0;
}
