#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

//Comandos ao servidor
#define GET "get"
#define REMOVE "remove"
#define ENCERRAR "encerrar"

//Comandos P2P




typedef struct no{
    char *telefone;
    char *nome;
    struct sockaddr_in localizacao;
    struct no *prox;
    struct no *ant;
}contato;

typedef struct no{
    contato *membros;
    int qtd;
    char *nome;
    struct no *prox;
    struct no *ant;
}grupo;

//Thread mensagens setup
typedef struct{
    int socket;
    contato *contatos;
    grupo *grupos;
}thread_arg,*ptr_thread_arg;

pthread_mutex_t mutex;

void *thread_msg(void*);

/*Remove usuario da lista, retorna 1 se sucesso*/
int remove_contato(contato **raiz,char *tel);
/*Adiciona o usuario no fim da lista, caso usuario ja exista nada eh feito*/
void adiciona_contato(contato **raiz,contato add);


/*Remove grupo da lista, retorna 1 se sucesso*/
int remove_grupo(grupo **raiz,char *nome);
/*Adiciona o usuario no fim da lista, caso usuario ja exista nada eh feito*/
void cria_grupo(grupo **raiz,grupo add);
/*Adiciona o usuario no fim da lista, caso usuario ja exista nada eh feito*/
void adiciona_membro(grupo **raiz,contato add,char *nome);
/*Remove usuario da lista, retorna 1 se sucesso*/
int remove_membro(grupo **raiz,char *tel);


/*printa a lista
void print_lista(grupo *lista){
    while(lista!=NULL){
        printf("Telefone: %s\tStatus: %i\n",lista->telefone,lista->conectado);
        lista = lista->prox;
    }
}
*/


int main(int argc, char *argv[])
{
    int operacao;

    char buffer_envio[50],aux[50];
    char buffer_recebimento[50];
    char telefone[20];

    struct hostent *hostnm;
    int inetaddr;
    struct sockaddr_in server,myself;

    grupo *grupos=NULL;
    contato *contatos=NULL;

    int t_creat_res;
    pthread_t ptid;
    thread_arg t_arg;
    if(pthread_mutex_init(&mutex,NULL)!=0){
        perror("ERRO - mutex_init()");
        exit(errno);
    } 

    int socket_recebe, socket_envia;
    int ip_recebimento, porta_recebimento;

    //Setup do socket de recebimento de mensagens
    if((socket_recebe = socket(PF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - Socket(recv)");
        exit(errno);
    }

    myself.sin_addr.s_addr = INADDR_ANY;
    myself.sin_family = AF_INET;
    srand(time(NULL));
    do{

        porta_recebimento = rand()%65500;
        myself.sin_port = htons(porta_recebimento);
        
    }while(bind(socket_recebe,(struct sockaddr *)&myself,sizeof(myself)) != 0);

    if(listen(socket_recebe,1) != 0){
        perror("ERRO - Listen()");
        exit(errno);
    }



    if (argc < 3){
        printf("Use %s <host> <porta>\n",argv[0]);
        exit(-1);
    }        
    
    //Setup socket conexao ao servidor
    hostnm = gethostbyname(argv[1]);
    inetaddr = inet_addr(argv[1]);
        
        server.sin_family=AF_INET;
        server.sin_port = htons(atoi(argv[2]));
        if( hostnm == (struct hostent *) 0)
        {
            if(inetaddr == INADDR_NONE)
            {
                fprintf(stderr,"ERRO - nome do servidor\n");
                exit(-1);
            }
            server.sin_addr.s_addr=inetaddr;
        }else server.sin_addr.s_addr=*((unsigned long *)hostnm->h_addr_list[0]);

    if ((socket_envia = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("ERRO - Socket(snd)");
        exit(errno);
    }

    if ((connect(socket_envia, (struct sockaddr *)&server, sizeof(server))) < 0){
        perror("ERRO - Connect()");
        exit(errno);
    }

    printf("Informe seu telefone: ");
    fgets(telefone,sizeof(telefone),stdin);

    strcat(buffer_envio,telefone);
    strcat(buffer_envio,";");
    sprintf(aux,"%d",porta_recebimento);
    strcat(buffer_envio,aux);
    if (send(socket_envia, buffer_envio, sizeof(buffer_envio), 0) < 0){
        perror("ERRO - send(servidor)");
        exit(errno);
    }
    //Conectado ao servidor, lancando thread de mensagens

    t_arg.contatos = contatos;
    t_arg.grupos = grupos;
    t_arg.socket = socket_recebe;

    t_creat_res = pthread_create(&ptid,NULL,&thread_msg,&t_arg);
    if(t_creat_res != 0){
        perror("ERRO - thread_create()");
        exit(t_creat_res);
    }

    do
    {
        printf("WhatsAp2p\n");
        printf("1 - Enviar Mensagem. \n");
        printf("2 - Criar Grupo. \n");
        printf("3 - Contatos. \n");
        printf("4 - Creditos.\n");
        printf("0 - sair. \n");
        scanf("%i", &operacao);
        //system("clear");

        switch (operacao)
        {
        case 1: //Enviar Msg
        {
        }
        break;

        case 2: //Criar Grupo
        {
        }
        break;

        case 3: //Adc Contato
        {
        }
        break;

        case 4: //Creditos
        {
        }
        break;

        default:
            printf("Nao entendi o que voce quer!\n");
            break;
        }

    } while (operacao != 0);


    if(send(socket_envia,ENCERRAR,sizeof(ENCERRAR),0)<0){
        perror("ERRO - send(ENCERRAR)");
    }
    pthread_cancel(ptid);
    close(socket_recebe);
    close(socket_envia);
}


int searchContato(contato **raiz,char *tel){

    contato *aux = *raiz;
    while(aux != NULL){
        if(strcmp(aux->telefone,tel) == 0){
            return 1;
        }else{
            aux = aux->prox;
        }
    }
    return 0;
}

int searchGrupo(grupo **raiz,char *nome){

    grupo *aux = *raiz;
    while(aux != NULL){
        if(strcmp(aux->nome,nome) == 0){
            return 1;
        }else{
            aux = aux->prox;
        }
    }
    return 0;
}

int remove_contato(contato **raiz,char *tel){

    contato *aux = *raiz;
    while(aux != NULL){
        if(strcmp(aux->telefone,tel) == 0){
            if(aux->ant == NULL){
                *raiz = aux->prox;
                free(aux);            
            }else{
                aux->ant->prox = aux->prox;
                free(aux);
            }
            return 1;
        }else{
            aux = aux->prox;
        }
    }
    return 0;
}

int remove_grupo(grupo **raiz,char *nome){

    grupo *aux = *raiz;
    while(aux != NULL){
        if(strcmp(aux->nome,nome) == 0){
            if(aux->ant == NULL){
                *raiz = aux->prox;
                free(aux);            
            }else{
                aux->ant->prox = aux->prox;
                free(aux);
            }
            return 1;
        }else{
            aux = aux->prox;
        }
    }
    return 0;
}

void adiciona_contato(contato **raiz,contato add){

    if(searchContato(raiz,add.telefone) == 1){
        return;
    }

    contato *novo = (contato *)malloc(sizeof(contato));
    if(novo == NULL){
        perror("ERRO - malloc(novo)");
        exit(errno);
    }

    novo->telefone = add.telefone;
    novo->nome = add.nome;
    novo->localizacao = add.localizacao;
    novo->prox=NULL;
        
    if(*raiz == NULL){
        *raiz = novo;
        (*raiz)->ant = NULL;
    }else{
        contato *aux = *raiz;
        while(aux->prox != NULL){
            aux = aux->prox;
        }
        aux->prox = novo;
        aux->prox->ant=aux;
    }
}

void adiciona_membro(grupo **raiz,contato add,char *nome){

    contato *novo = (contato *)malloc(sizeof(contato));
    if(novo == NULL){
        perror("ERRO - malloc(novo)");
        exit(errno);
    }
    novo->telefone = add.telefone;
    novo->nome = add.nome;
    novo->localizacao = add.localizacao;
    novo->prox=NULL;


    while(*raiz != NULL){

        if(strcmp((*raiz)->nome,nome) == 0){
            if(searchContato((*raiz)->membros,add.telefone) == 1){
                free(novo);
                return;
            }
            contato *mem = (*raiz)->membros;

            if(mem == NULL){
                mem = novo;
                mem->ant = NULL;
            }else{
                while(mem->prox != NULL){
                    mem = mem->prox;
                }
                mem->prox = novo;
                mem->prox->ant=mem;   
            }
            (*raiz)->qtd+=1;
            return;
        }else{
            (*raiz) = (*raiz)->prox;
        }
    }

}

void cria_grupo(grupo **raiz,grupo add){

    if(searchGrupo(raiz,add.nome) == 1){
        return;
    }

    grupo *novo = (grupo *)malloc(sizeof(grupo));
    if(novo == NULL){
        perror("ERRO - malloc(novo)");
        exit(errno);
    }
    novo->nome = add.nome;
    novo->membros = add.membros;
    novo->qtd = add.qtd;
    novo->prox=NULL;
        
    if(*raiz == NULL){
        *raiz = novo;
        (*raiz)->ant = NULL;
    }else{
        grupo *aux = *raiz;
        while(aux->prox != NULL){
            aux = aux->prox;
        }
        aux->prox = novo;
        aux->prox->ant=aux;
    }
}