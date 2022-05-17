#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include <sys/mman.h>
#include<unistd.h>

#define THREAD_NUM 4
#define DELAY 5
#define BUFFER_SIZE 1964
#define READ_END 0
#define WRITE_END 1

char writeMessage[BUFFER_SIZE]="Kara Delik Nedir? Kara Delik Nasıl Oluşur?\nKara delikler, uzayda yol alan hiçbir madde veya radyasyonun kaçamayacağı kadar büyük kütleçekim alanlarıdır. Aslında kara delikler, ölü yıldızlardır;\nçünkü büyük kütleli bir yıldızın yakıtı bittiğinde, kendi üzerine çöker ve bir kara delik oluşturur.\nKara Delik Nasıl Oluşur?\nBir yıldızı, devasa bir termonükleer reaktör olarak düşünebilirsiniz. Bu reaktörün yakıtı, yıldızın çekirdeğinde süregelen füzyon tepkimeleridir.\nBu tip tepkimede, hidrojen gibi daha küçük atom numarasına sahip elementler, birbirine kaynayarak helyum gibi daha büyük atom numaralı elementlere dönüşürler.\nBu kaynaşma sırasında etrafa bol miktarda enerji saçılır. Bu enerji, yıldızın içindeki atomları dışarıya doğru iter.\nAncak atomların etrafa saçılarak yıldızın dağılmamasına neden olan bir diğer kuvvet vardır: kütleçekimi.\nAtomlar arası içe doğru olan çekim kuvveti, bu füzyon tepkimesinin dışa doğru olan kuvvetini dengeler. Böylece yıldız, hidrostatik denge adı verilen bir denge halinde kalır.\nHer ne kadar kütleçekimi (bildiğimiz kadarıyla) tükenebilen bir olgu değilse de, füzyon tepkimesi sonsuz değildir.\nYıldızlar, kendilerinden önce gelen gaz ve toz bulutu içinde (nebulalarda) oluşurlar.\nNebulalar ise daha önceden ömürlerini tamamlamış yıldızların etrafa saçtıkları gaz ve toz bulutlarıdır. Bu gaz ve toz bulutu içinde belli miktarda hidrojen atomu bulunur;\nbu atomların sayısı sonsuz değildir. Dolayısıyla bir nebula içinde oluşan yıldızın tüketebileceği hidrojen miktarı da sınırlıdır.\nİşte bir süre sonra yıldız, hidrojen yakıtlarını tüketir. Böylece füzyon tepkimesi giderek yavaşlar; ancak kütleçekiminin etkisi değişmez.\nKütleçekimi ağır bastıkça, hidrostatik denge bozulmaya başlar ve yıldız kendi içine doğru çökmeye başlar.";
char readMessage[BUFFER_SIZE];

void* create_shared_memory(size_t size) { //Shared memory oluştturması
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    return mmap(NULL, size, protection, visibility, -1, 0);
}

void* routine(void* arg){ //Thread kullanımı ve sezar şifrelemesi
    int index = *(int*) arg;
    char* message =  malloc((int)(BUFFER_SIZE/THREAD_NUM));
    sleep(1);
    for(int j=0;j<(int)(BUFFER_SIZE/THREAD_NUM);j++){
        message[j]= readMessage[index + j];
    }
    
    char * ciphered=malloc((int)(BUFFER_SIZE/THREAD_NUM));
    char character;
    
    for(int i=0;i<strlen(message);i++) //sezar şifreleme
    {
        if(message[i]==' ')
        {
            ciphered[i]=' ';
            continue;
        }   
        else
        {
            character=(char) (((int) message[i]-65+DELAY)%26+65); 
            ciphered[i]=character;
        }
    }
    return ciphered;
}

int main(void)
{
    int fileDescriptor[2];
    pid_t pid;
    void* sharedMemory=create_shared_memory(128);
    memcpy(sharedMemory,writeMessage,sizeof(writeMessage));
    if (pipe(fileDescriptor) == -1)
    {
        fprintf(stderr, "PIPE COULD NOT BE CREATED...");
        exit(1);
    }
	pid = fork();

	if (pid < 0)
	{
		fprintf(stderr, "FORK COULD NOT BE CREATED...\n");
		exit(1);
	}
	if(pid == 0)//child
	{
        close(fileDescriptor[WRITE_END]);
        read(fileDescriptor[READ_END], readMessage, BUFFER_SIZE);
        close(fileDescriptor[READ_END]);
        pthread_t th[THREAD_NUM];
        char* returnCiphered;
        char mergeCipher[BUFFER_SIZE];
        for (int i = 0;i<THREAD_NUM;i++){
            int* a = malloc(sizeof(char));
            *a = i*(int)(BUFFER_SIZE/THREAD_NUM);
            if(pthread_create(&th[i], NULL, &routine,a)!= 0){
                perror("Failed to create thread");
            }
            pthread_join(th[i],(void**) &returnCiphered);
            strcat(mergeCipher,returnCiphered);
        }
        memcpy(sharedMemory,mergeCipher,sizeof(mergeCipher));
	}
	else//parent
	{
        close(fileDescriptor[READ_END]);
        write(fileDescriptor[WRITE_END], writeMessage, strlen(writeMessage)+1);
        close(fileDescriptor[WRITE_END]);
        wait(NULL);
        sleep(1);
        printf("%s",sharedMemory);
	}
}
