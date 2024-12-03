#include <cstdio>
#include <cstdlib>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <sys/wait.h>

extern "C" {
#include "analyst/include/analyst.h"
}

// Helper function to create temporary test files
std::string create_temp_file(const std::vector<pid_t> &data) {
  char temp_filename[] = "/tmp/testfileXXXXXX";
  int fd = mkstemp(temp_filename);
  if (fd == -1) {
    perror("mkstemp failed");
    exit(EXIT_FAILURE);
  }
  FILE *file = fdopen(fd, "wb");
  fwrite(data.data(), sizeof(pid_t), data.size(), file);
  fclose(file);
  return std::string(temp_filename);
}

FILE *create_empty_temp_file() {
  char temp_filename[] = "/tmp/testfileXXXXXX";
  int fd = mkstemp(temp_filename);
  if (fd == -1) {
    perror("mkstemp failed");
    exit(EXIT_FAILURE);
  }
  FILE *file = fdopen(fd, "wb");
  return file;
}

std::string read_file_content(FILE *file) {
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  if (file_size < 0) {
    throw std::runtime_error("Failed to determine file size.");
  }

  std::string content(static_cast<size_t>(file_size), '\0');

  rewind(file);

  size_t read_size = fread(content.data(), 1, content.size(), file);
  if (read_size != content.size()) {
    throw std::runtime_error("Failed to read the entire file content.");
  }

  return content;
}

TEST(AnalystTest, ValidInputMoreThanMax) {
  // Arrange
  const uint8_t max_numbers = 3;
  std::vector<pid_t> data = {10, 20, 30, 40, 50};
  std::string temp_file = create_temp_file(data);
  FILE *output_stream = create_empty_temp_file();
  std::string expected_output = "10\n20\n30\n40\n50\n";

  Analyst analyst = create_analyst(const_cast<char *>(temp_file.c_str()),
                                   max_numbers, output_stream);
  // Act
  pid_t pid = create_analyst_process(&analyst);

  // Assert
  int status;
  waitpid(pid, &status, WUNTRACED);
  ASSERT_TRUE(WIFSTOPPED(status));

  kill(pid, SIGCONT);

  waitpid(pid, &status, WUNTRACED);
  ASSERT_TRUE(WIFSTOPPED(status));

  kill(pid, SIGCONT);

  waitpid(pid, &status, 0);
  ASSERT_TRUE(WIFEXITED(status));
  ASSERT_EQ(WEXITSTATUS(status), EXIT_SUCCESS);

  std::string actual_output = read_file_content(output_stream);
  ASSERT_EQ(actual_output, expected_output);

  unlink(temp_file.c_str());
}

// TEST(AnalystTest, ValidInputLessThanMax) {
//   const uint8_t max_numbers = 5;
//   std::vector<pid_t> data = {10, 20};
//   std::string temp_file = create_temp_file(data);
//
//   Analyst analyst =
//       create_analyst(const_cast<char *>(temp_file.c_str()), max_numbers);
//   pid_t pid = create_analyst_process(&analyst);
//
//   int status;
//   waitpid(pid, &status, 0);
//   ASSERT_TRUE(WIFEXITED(status));
//   ASSERT_EQ(WEXITSTATUS(status), EXIT_SUCCESS);
//
//   unlink(temp_file.c_str());
// }
//
// TEST(AnalystTest, EmptyFile) {
//   const uint8_t max_numbers = 5;
//   std::string temp_file = create_temp_file({}); // Empty file
//
//   Analyst analyst =
//       create_analyst(const_cast<char *>(temp_file.c_str()), max_numbers);
//   pid_t pid = create_analyst_process(&analyst);
//
//   int status;
//   waitpid(pid, &status, 0);
//   ASSERT_TRUE(WIFEXITED(status));
//   ASSERT_EQ(WEXITSTATUS(status), EXIT_SUCCESS);
//
//   unlink(temp_file.c_str());
// }
//
// TEST(AnalystTest, InvalidFilePath) {
//   const uint8_t max_numbers = 5;
//
//   ASSERT_DEATH(
//       {
//         Analyst analyst = create_analyst(nullptr, max_numbers);
//         create_analyst_process(&analyst);
//       },
//       "empty lng file path");
// }
//
// TEST(AnalystTest, InvalidFileOpen) {
//   const uint8_t max_numbers = 5;
//
//   ASSERT_DEATH(
//       {
//         Analyst analyst = create_analyst(
//             const_cast<char *>("/nonexistent_file"), max_numbers);
//         create_analyst_process(&analyst);
//       },
//       "failed to open lng file");
// }
