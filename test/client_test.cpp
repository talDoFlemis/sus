#include "gtest/gtest.h"
#include <stdint.h>

extern "C" {
#include "client/include/client.h"
#include "utils/include/time.h"
}

TEST(ClientTest, SleepsAndExitsSuccessfully) {
  // Arrange
  const uint64_t sleep_time = 1000; // Sleep time in microseconds
  // Act
  pid_t pid = fork();

  // Assert
  ASSERT_GE(pid, 0); // Ensure fork was successful
  if (pid == 0) {
    // Child process
    client(sleep_time);
  } else {
    // Parent process
    int status;
    struct timespec start_time, end_time;
    get_now(&start_time);

    // Wait for the child process to terminate
    pid_t result = waitpid(pid, &status, 0);
    double elapsed_time_in_us =
        (double)elapsed_time_until_now(start_time) / 1000;

    ASSERT_EQ(result, pid); // Ensure the correct child process is observed
    ASSERT_TRUE(WIFEXITED(status)); // Check if the child process exited
    ASSERT_EQ(WEXITSTATUS(status),
              EXIT_SUCCESS); // Verify it exited successfully

    // Verify the child process slept approximately for the expected time
    ASSERT_NEAR(elapsed_time_in_us, sleep_time, 10e2);
  }
}
