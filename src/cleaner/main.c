#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  printf("Cleaning up semaphores...\n");
  sem_unlink("/sem_atend");
  sem_unlink("/sem_block");
  sem_unlink("/sem_scheduler");

  printf("Cleaning up demand file...\n");
  remove("demanda.txt");

  return EXIT_SUCCESS;
}
