#include "gtest/gtest.h"

extern "C" {
#include "reception/include/client_factory.h"
}

TEST(ClientFactoryTest, CreateSingleClientProcess) {
  // Arrange
  ClientFactory factory = create_client_factory(200); // 200 us

  // Act
  ClientProcess client = create_client_process(factory);

  // Assert
  ASSERT_GT(client.pid, 0); // Ensure valid PID

  // Wait for process to stop
  int status;
  pid_t result = waitpid(client.pid, &status, WUNTRACED);
  ASSERT_EQ(result, client.pid);   // Correct child
  ASSERT_TRUE(WIFSTOPPED(status)); // Check stopped

  // Resume and wait for process to complete
  kill(client.pid, SIGCONT);
  result = waitpid(client.pid, &status, 0);
  ASSERT_TRUE(WIFEXITED(status));               // Ensure child exited
  ASSERT_EQ(WEXITSTATUS(status), EXIT_SUCCESS); // Verify success
}

TEST(ClientFactoryTest, CreateClientProcessesInBulk) {
  uint64_t num_processes = 5;
  ClientFactory factory = create_client_factory(100);

  // Act
  ClientProcess *clients =
      create_client_processe_in_bulk(factory, num_processes);

  // Assert
  ASSERT_NE(clients, nullptr);

  for (uint64_t i = 0; i < num_processes; ++i) {
    ASSERT_GT(clients[i].pid, 0); // Valid PID

    // Wait for process to stop
    int status;
    pid_t result = waitpid(clients[i].pid, &status, WUNTRACED);
    ASSERT_EQ(result, clients[i].pid);
    ASSERT_TRUE(WIFSTOPPED(status));

    // Resume and wait for process to complete
    kill(clients[i].pid, SIGCONT);
    result = waitpid(clients[i].pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), EXIT_SUCCESS);
  }

  free(clients);
}
