//������ص�ͷ�ļ�  
#include<termios.h>    /*PPSIX �ն˿��ƶ���*/ 
#include<stdio.h>      /*��׼�����������*/  
#include<stdlib.h>     /*��׼�����ⶨ��*/  
#include<unistd.h>     /*Unix ��׼��������*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*�ļ����ƶ���*/  
#include<errno.h>      /*����Ŷ���*/  
#include<string.h> 

//�궨��  
#define TRUE   0  
#define FALSE  -1  


/*******************************************************************
* ���ܣ�            ���ô�������λ��ֹͣλ��Ч��λ
* ��ڲ�����        fd         �����ļ�������
*                   speed      �����ٶ�
*                   flow_ctrl  ����������
*                   databits   ����λ   ȡֵΪ 7 ����8
*                   stopbits   ֹͣλ   ȡֵΪ 1 ����2
*                   parity     Ч������ ȡֵΪN,E,O,,S
*���ڲ�����         ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UARTx_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{

    int   i;
        int   speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
        int   name_arr[]  = { 115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300 };

    struct termios options;

        /* tcgetattr(fd,&options)�õ���fdָ��������ز������������Ǳ�����options,
         * �ú���,�����Բ��������Ƿ���ȷ���ô����Ƿ���õȡ�
         * �����óɹ�����������ֵΪ0��������ʧ�ܣ���������ֵΪ1.
         */
    if( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");    
        return(FALSE); 
    }

    //���ô������벨���ʺ����������
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {       
            cfsetispeed(&options, speed_arr[i]); 
            cfsetospeed(&options, speed_arr[i]);  
            printf("uart_speed=%d ", speed );   
        }
    }     

    //�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���
    options.c_cflag |= CLOCAL;
    //�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������
    options.c_cflag |= CREAD;

    //��������������
    switch(flow_ctrl)
    {
        case 0 ://��ʹ��������
            options.c_cflag &= ~CRTSCTS;
                break;   

        case 1 ://ʹ��Ӳ��������
                options.c_cflag |= CRTSCTS;
                break;
        case 2 ://ʹ�����������
                options.c_cflag |= IXON | IXOFF | IXANY;
                break;
    }
    printf("flow_ctrl=%d ",flow_ctrl);


    //��������λ
    options.c_cflag &= ~CSIZE; //����������־λ
    switch (databits)
    {  
        case 5    :
            options.c_cflag |= CS5;
            break;
        case 6    :
            options.c_cflag |= CS6;
            break;
        case 7    :    
            options.c_cflag |= CS7;
            break;
        case 8:    
            options.c_cflag |= CS8;
            break;  
        default:   
            fprintf(stderr,"Unsupported data size\n");
            return (FALSE); 
    }
    printf("databits=%d ",databits);


    //����У��λ
    switch (parity)
    {  
        case 'n':
        case 'N': //����żУ��λ��
            options.c_cflag &= ~PARENB; 
            options.c_iflag &= ~INPCK;  
            printf("parity=N ");  
            break; 
        case 'o':  
        case 'O'://����Ϊ��У��
            options.c_cflag |= (PARODD | PARENB); 
            options.c_iflag |= INPCK;             
            break; 
        case 'e': 
        case 'E'://����ΪżУ��
            options.c_cflag |= PARENB; 
            options.c_cflag &= ~PARODD;       
            options.c_iflag |= INPCK;       
            break;
        case 's':
        case 'S': //����Ϊ�ո�
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break; 
        default:  
            fprintf(stderr,"Unsupported parity\n");   
            return (FALSE); 
    } 


    // ����ֹͣλ 
    switch (stopbits)
    {  
        case 1:   
            options.c_cflag &= ~CSTOPB; 
            break; 
        case 2:   
            options.c_cflag |= CSTOPB; 
            break;
        default:   
            fprintf(stderr,"Unsupported stop bits\n"); 
            return (FALSE);
    }
    printf("stopbits=%d \n",stopbits);


    //�޸����ģʽ��ԭʼ������� /*Output*/
    options.c_oflag &= ~OPOST;

    //options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//�Ҽӵ� /*Input*/ 
    options.c_lflag &= ~(ISIG | ICANON);  

    //���õȴ�ʱ�����С�����ַ�
    options.c_cc[VTIME] = 0; /* ��ȡһ���ַ��ȴ�1*(1/10)s */  
    options.c_cc[VMIN] = 1;  /* ��ȡ�ַ������ٸ���Ϊ1 */

    //������??����������������ݣ����ǲ��ٶ�ȡ
    tcflush(fd,TCIFLUSH);

    //�������� (���޸ĺ��termios�������õ������У�
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
    {
        perror("com set error!\n");  
        return (FALSE); 
    }

    return (TRUE); 
}


/*******************************************************************
* ���ƣ�           UART0_Recv
* ���ܣ�           ���մ�������
* ��ڲ�����       fd          :�ļ�������    
*                  rcv_buf     :���մ��������ݴ���rcv_buf�������??*                  data_len    :һ֡���ݵĳ���
* ���ڲ�����       ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UARTx_Recv(int fd, char *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    //ʹ��selectʵ�ִ��ڵĶ�·ͨ��
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    if(fs_sel)
    {
            len = read(fd,rcv_buf,data_len);
        printf("I am right!(version1.2) len = %d fs_sel = %d\n",len,fs_sel);  
            return len;
    }
    else
    {
        printf("Sorry,I am wrong!\n");  
            return FALSE;
    }     
}

/*******************************************************************
* ���ƣ�            UART0_Send
* ���ܣ�            ��������
* ��ڲ�����         fd         :�ļ�������    
*                   send_buf    :�??�Ŵ��ڷ�������
*                   data_len    :һ֡���ݵĸ��??
* ���ڲ�����        ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UARTx_Send(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    if (len == data_len ){
        return len;
    }     
    else{
        tcflush(fd,TCOFLUSH);
        return FALSE;
    }
}


/*****************************************************************
* ���ܣ�         �򿪴��ڲ����ش����豸�ļ�����
* ��ڲ�����     fd:�ļ�������   port:���ں�(ttyS0,ttyS1,ttyS2)
* ���ڲ�����     ��ȷ����Ϊ1�????�󷵻�Ϊ0
*****************************************************************/
int UARTx_Open(int fd, const char * ttyX)
{
    //fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(ttyX, O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd == FALSE)
    {
        perror("open_port: Unable to open /dev/ttyS0 -");
        return(FALSE);
    }

    //�жϴ��ڵ�״̬�Ƿ�Ϊ����״̬
    if( fcntl(fd,F_SETFL,0)<0 ){
        printf("fcntl failed! \n");
        return(FALSE);
    }     
    else{
        printf("fcntl=%d \n",fcntl(fd, F_SETFL,0));
    }

    //�����Ƿ�Ϊ�??���豸    
    if(0 == isatty(STDIN_FILENO) ){
        printf("standard input is not a terminal device \n");
        return(FALSE);
    }
    else{
        printf("isatty success!\n");
    }
    printf("fd->open=%d \n",fd);

    return (fd);
}


/******************************************************
* ���ƣ�    main
*******************************************************************/
int main(int argc, char **argv)  
//int main(void)  
{  
    int fd;                            //�ļ�������  
    int err;                           //���ص��ú�����״̬  
    int len;                          
    int i;  

    char rcv_buf[256];         
    char send_buf[20]="uartx_test\n";  

    //fd = UART0_Open(fd,argv[1]); //�򿪴��ڣ������ļ�������  
    //fd = UARTx_Open(fd); //�򿪴��ڣ������ļ�������  
    fd = UARTx_Open(fd,argv[1]);
    if(FALSE == fd){        
        exit(1);    
    }

    //���ô��ڲ��??    err = UARTx_Set(fd,57600,0,8,1,'N');  
    if (FALSE == err){  
        printf("Set Port Error\n"); 
        exit(1); 
    }
   
    if(!strcmp(argv[2], "Head Up"))
    {
	memcpy(send_buf, "5A 10 10 02 40 00", sizeof("5A 10 10 02 40 00"));
    }


    for( i = 0; i < 10; i++ )  
    {  
        len = UARTx_Send(fd, send_buf, strlen(send_buf));  
        if(len > 0)  
            printf(" %d send data successful len=%d \n",i,len);  
        else  
            printf("send data failed!\n"); 
    }

    len = 0 ;
    while(1) 
    {
        len = read(fd, rcv_buf, 256);  

        if(len > 0){
            rcv_buf[len] = '\n';
            UARTx_Send(fd,rcv_buf,len+1);
            printf("len=%d uart_RX_ok \n", len); 

            len = 0 ;
            memset(rcv_buf, 0, sizeof(rcv_buf) );
        } 
        //sleep(2); 
    } 

        printf("Close...\n");
    close(fd);   

    return 0;
}
