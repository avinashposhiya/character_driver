/***************************************************************************//**
*  File       application.c
*
*  Details    Userspace application to test the Device driver
*
*  Author     Avinash Poshiya
*
*  Tested with Linux Beagle Bone Black and Ubuntu22.04 5.15.0-105-generic
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
#define APPLICATION_MEM_SIZE 2048
 
int main()
{
        int fd;
        int32_t value, number;
        char write_buf[APPLICATION_MEM_SIZE] = {0};
	char read_buf[APPLICATION_MEM_SIZE] = {0};
        
        printf("*******Welcom to the test application*******\n");
 
        printf("Opening the Driver\n");
        fd = open("/dev/my_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file.\n");
                return 0;
        }
 
 	printf("\n***Validate ioctl functionality start***\n");
 	
        printf("Enter the Value to send\n");
        scanf("%d",&number);
        printf("WR_VALUE ioctl call for write Value to my Driver\n");
        ioctl(fd, WR_VALUE, (int32_t*) &number); 
 
        printf("RD_VALUE ioctl call for read Value from my Driver\n");
        ioctl(fd, RD_VALUE, (int32_t*) &value);
        printf("Value is %d\n", value);
        
        printf("\n*********************************************\n");
        while(1){
 		printf("***Validate Read/Write functionality start***\n");
 	
 		printf("***Validate ioctl functionality end***\n");
 		printf("Enter the string to write into driver :");
        	scanf("  %[^\t\n]s", write_buf);
        	printf("Data Writing to the Driver...\n");
        	write(fd, write_buf, strlen(write_buf)+1);
        
        	printf("\nData Reading from the driver...\n");
        	read(fd, read_buf, 1024);
        	printf("Data Read from the driver = %s\n", read_buf);
 	}//end of while loop

 	printf("\n***Validate Read/Write functionality start***\n");
 	
 	
 	/*Close the driver*/
        printf("Closing the Driver\n");
        close(fd);
}
