/*
Trabalho prático 1
Disciplina: Estrutura de dados e algoritmos 2
Professor: Flavio Assis
Aluno: Eduardo Rodrigues Seifert

Algumas considerações:
1. O programa solta runtime error caso o valor da constante TAMANHO_ARQUIVO é mudada e database.bin já existe e tem em seu header um tamanho diferente.
2. O programa solta runtime error caso tente-se inserir um elemento em um arquivo cheio
3. Caso o arquivo esteja cheio, o ponteiro para a primeira posicao vazia será nulo
4. Uma melhoria a ser feita: encapsular funcoes em arquivos distintos, usando makefile(não foi feito por causa da minha falta de familiaridade com o makefile, mas se me for permitido fazer depois do prazo de entrega, farei)
*/

#include<bits/stdc++.h>
#include<stdlib.h>
#include<unistd.h>

using namespace std;

#define tam TAMANHO_ARQUIVO
#define TAMANHO_ARQUIVO 11

FILE *arq;
int controles[4];
double media = 0;

struct reg{
	int prox,anterior,chave,tamcad,indice;
	char dado[21];
	bool vazio,ishead;
};

int h(int chave){
	return chave%tam;
}

bool isEmpty(){
	if(access("database.bin",F_OK)==-1){
		return true;
	}
	return false;
}

void writeHeader(){
	fseek(arq, 0, SEEK_END);
	fwrite(&media,sizeof(double),1,arq);
	fwrite(controles,sizeof(int),4,arq);
}

void initialize(){
	arq = fopen("database.bin","wb");
	for(int i=0;i<tam;i++){
		reg novo;
		novo.anterior = i+1;
		if(i==tam-1)novo.anterior = -1;
		novo.prox = i-1;
		novo.chave = i;
		novo.dado[0]='\0';
		novo.vazio=true;
		novo.tamcad=0;
		novo.ishead=false;
		novo.indice = i;
		fwrite(&novo, sizeof(reg),1,arq);
	}
	controles[0] = tam-1;
	controles[1] = 0;
	controles[2] = 0;
	controles[3] = tam;
	writeHeader();
	fclose(arq);
}

void printFile(){	
	fseek(arq,0,SEEK_SET);
	for(int i=0;i<tam;i++){
		reg atual;
		fread(&atual,sizeof(reg),1,arq);
		cout<<atual.indice<<": ";
		if(atual.vazio){
		cout<< "apontador1: "+ (atual.anterior == -1 ? "nulo " : to_string(atual.anterior)+" ")+ "apontador2: "+ (atual.prox == -1 ? "nulo " : to_string(atual.prox));
		}
		else{
			cout<<atual.chave<<" "<<atual.dado<<" "<<(atual.prox == -1 ? "nulo" : to_string(atual.prox));
		}
		cout<<endl;
	}
	cout<<"posicao inicial da lista de posicoes vazias: "<<(controles[0]==-1 ? "nulo" : to_string(controles[0]))<<endl;
}

void stringToArrayChar(char* regDado,string dado){
	for(unsigned int i=0;i<dado.size();i++){
		regDado[i]=dado[i];
	}
	regDado[dado.size()]='\0';
}

void atualizaListaVazia(int anterior, int prox){
	reg atualiza;
	if(anterior!=-1){
		fseek(arq,sizeof(reg)*anterior,SEEK_SET);
		fread(&atualiza,sizeof(reg),1,arq);
		fseek(arq,-sizeof(reg),SEEK_CUR);
		atualiza.prox=prox;
		fwrite(&atualiza,sizeof(reg),1,arq);
	}
	if(prox!=-1){
		fseek(arq,sizeof(reg)*prox,SEEK_SET);
		fread(&atualiza,sizeof(reg),1,arq);
		fseek(arq,-sizeof(reg),SEEK_CUR);
		atualiza.anterior=anterior;
		fwrite(&atualiza,sizeof(reg),1,arq);
	}
}

pair<int,bool> buscaChave(int chave){
	pair<int,bool> p;
	p.second=false;
	int hashed = h(chave);
	reg seeker;
	fseek(arq, sizeof(reg)*hashed,SEEK_SET);
	fread(&seeker, sizeof(reg),1, arq);
	while(seeker.chave!=chave&&seeker.prox!=-1){
		fseek(arq, sizeof(reg)*seeker.prox,SEEK_SET);
		fread(&seeker, sizeof(reg),1, arq);
	}
	p.first=seeker.indice;
	if(seeker.chave==chave&&seeker.vazio==false)p.second=true;
	return p;
}

bool insereReg(int chave, string dado){
	if(controles[1]==controles[3]){
		throw runtime_error("Insercao em arquivo cheio: Impossivel");
		exit(1); 
	}
	bool altereicabeca=false;
	pair<int,bool> posicao;
	int resh = h(chave);
	fseek(arq, sizeof(reg)*resh,SEEK_SET);
	reg novo;
	fread(&novo, sizeof(reg),1, arq);
	if(novo.vazio){
		int anterior = novo.anterior;
		int prox = novo.prox;
		novo.anterior = -1;
		novo.prox = -1;
		novo.tamcad = 1;
		stringToArrayChar(novo.dado,dado);
		novo.chave = chave;
		novo.ishead=true;
		novo.vazio=false;
		if(resh == controles[0]){
			controles[0] = prox;
			altereicabeca=true;
		}
		fseek(arq,-sizeof(reg),SEEK_CUR);
		fwrite(&novo,sizeof(reg),1,arq);
		controles[1]++;
		controles[2]++;
		atualizaListaVazia(anterior,prox);
	}
	else{
		posicao = buscaChave(chave);
		if(posicao.second){
			return false;
		}	
		else if(h(novo.chave)!=resh){
			novo.indice=controles[0];
			reg atualiza;
			fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
			fread(&atualiza,sizeof(reg),1,arq);
			controles[0]=atualiza.prox;
			fseek(arq,sizeof(reg)*novo.indice,SEEK_SET);
			fwrite(&novo,sizeof(reg),1,arq);
			fseek(arq,sizeof(reg)*novo.anterior,SEEK_SET);
			fread(&atualiza,sizeof(reg),1,arq);
			fseek(arq,sizeof(reg)*novo.anterior,SEEK_SET);
			atualiza.prox = novo.indice;
			fwrite(&atualiza,sizeof(reg),1,arq);
			novo.anterior=-1;
			novo.prox=-1;
			novo.tamcad = 1;
			stringToArrayChar(novo.dado, dado);
			novo.chave=chave;
			novo.ishead = true;
			novo.vazio = false;
			controles[1]++;
			controles[2]++;
			novo.indice = resh;
			fseek(arq,sizeof(reg)*resh,SEEK_SET);
			fwrite(&novo,sizeof(reg),1,arq);
			altereicabeca=true;
		}
		else{
			bool segundo=false;
			reg head = novo;
			head.tamcad++;
			controles[1]++;
			controles[2]+=head.tamcad;
			if(posicao.first==head.indice){
				head.prox=controles[0];
				segundo=true;
			}
			fseek(arq,sizeof(reg)*head.indice,SEEK_SET);
			fwrite(&head,sizeof(reg),1,arq);
			novo.ishead=false;
			novo.vazio=false;
			novo.anterior=posicao.first;
			novo.prox=-1;
			novo.chave=chave;
			novo.indice=controles[0];
			stringToArrayChar(novo.dado,dado);
			reg atualiza;
			fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
			fread(&atualiza,sizeof(reg),1,arq);
			controles[0]=atualiza.prox;
			fseek(arq,-sizeof(reg),SEEK_CUR);
			fwrite(&novo,sizeof(reg),1,arq);
			if(!segundo){
				fseek(arq,sizeof(reg)*posicao.first,SEEK_SET);
				fread(&atualiza,sizeof(reg),1,arq);
				atualiza.prox=novo.indice;
				fseek(arq,-sizeof(reg),SEEK_CUR);
				fwrite(&atualiza,sizeof(reg),1,arq);
			}
			altereicabeca=true;
		}
	}
	media = (double) controles[2]/(double)controles[1];
	if(altereicabeca){
		fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
		fread(&novo,sizeof(reg),1,arq);
		novo.anterior=-1;
		fseek(arq,-sizeof(reg),SEEK_CUR);
		fwrite(&novo,sizeof(reg),1,arq);
	}
	return true;
	
}

bool removeReg(int chave){
	pair<int, bool> p = buscaChave(chave);
	if(p.second==false)return false;
	int resh = h(chave);
	reg head,atualiza;
	fseek(arq,sizeof(reg)*resh,SEEK_SET);
	fread(&head,sizeof(reg),1,arq);
	controles[1]--;
	if(head.indice==p.first){
		if(head.tamcad==1){
			head.vazio=true;
			head.anterior = -1;
			head.prox=controles[0];
			fseek(arq,sizeof(reg)*head.indice,SEEK_SET);
			fwrite(&head,sizeof(reg),1,arq);
			fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
			fread(&atualiza,sizeof(reg),1,arq);
			atualiza.anterior=head.indice;
			fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
			fwrite(&atualiza,sizeof(reg),1,arq);
			controles[2]--;
			controles[0]=head.indice;		
		}
		else{
			fseek(arq,sizeof(reg)*head.prox,SEEK_SET);
			fread(&atualiza,sizeof(reg),1,arq);
			head.vazio=true;
			head.anterior = -1;
			head.prox=controles[0];
			atualiza.ishead=true;
			atualiza.anterior=-1;
			atualiza.tamcad=head.tamcad-1;
			int proxPosVazia = atualiza.indice;
			atualiza.indice=head.indice;		
			controles[2]-=head.tamcad;
			fseek(arq,sizeof(reg)*head.indice,SEEK_SET);
			cout<<head.indice<<endl;
			fwrite(&atualiza,sizeof(reg),1,arq);
			if(atualiza.prox!=-1){
				fseek(arq,sizeof(reg)*atualiza.prox,SEEK_SET);
				fread(&atualiza,sizeof(reg),1,arq);
				atualiza.anterior = head.indice;
				fseek(arq,sizeof(reg)*atualiza.indice,SEEK_SET);
				fwrite(&atualiza,sizeof(reg),1,arq);	 
			}
			fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
			fread(&atualiza,sizeof(reg),1,arq);
			atualiza.anterior=proxPosVazia;
			fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
			fwrite(&atualiza,sizeof(reg),1,arq);
			controles[0]=proxPosVazia;
			head.indice=controles[0];
			fseek(arq,sizeof(reg)*head.indice,SEEK_SET);
			fwrite(&head,sizeof(reg),1,arq);	
		}
	}
	else{
		fseek(arq,sizeof(reg)*p.first,SEEK_SET);
		fread(&atualiza,sizeof(reg),1,arq);
		controles[2]-=head.tamcad;
		fseek(arq,sizeof(reg)*head.indice,SEEK_SET);
		head.tamcad--;
		fwrite(&head,sizeof(reg),1,arq);
		int prox = atualiza.prox, anterior = atualiza.anterior;
		if(prox != -1){
			reg proxReg;
			fseek(arq,sizeof(reg)*prox,SEEK_SET);
			fread(&proxReg,sizeof(reg),1,arq);
			proxReg.anterior=anterior;
			fseek(arq,-sizeof(reg),SEEK_CUR);
			fwrite(&proxReg,sizeof(reg),1,arq);
		}
		if(anterior !=-1){
			reg proxReg;
			fseek(arq,sizeof(reg)*anterior,SEEK_SET);
			fread(&proxReg,sizeof(reg),1,arq);
			proxReg.prox=prox;
			fseek(arq,-sizeof(reg),SEEK_CUR);
			fwrite(&proxReg,sizeof(reg),1,arq);
		}
		atualiza.vazio=true;
		atualiza.anterior=-1;
		atualiza.prox=controles[0];
		int proxPosVazia = atualiza.indice;
		fseek(arq,sizeof(reg)*proxPosVazia,SEEK_SET);
		fwrite(&atualiza,sizeof(reg),1,arq);
		fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
		fread(&atualiza,sizeof(reg),1,arq);
		atualiza.anterior=proxPosVazia;
		fseek(arq,sizeof(reg)*controles[0],SEEK_SET);
		fwrite(&atualiza,sizeof(reg),1,arq);
		controles[0]=proxPosVazia;
	}
	media = (controles[1]!=0 ? (double) controles[2]/(double)controles[1] : 0.0);
	return true;	
}

void readHeader(){
	fseek(arq, -sizeof(int)*4 - sizeof(double), SEEK_END);
	int posIni, regOc, accTot, fileSize;
	double mediaTest;
	fread(&mediaTest,1,sizeof(double),arq);
	fread(&posIni,1,sizeof(int),arq);
	fread(&regOc,1,sizeof(int),arq);
	fread(&accTot,1,sizeof(int),arq);
	fread(&fileSize,1,sizeof(int),arq);
	if(fileSize!=tam){
		throw runtime_error("Tamanho de arquivo diferente do esperado. Esperado =  " + to_string(tam) + " recebido = " + to_string(fileSize));
		exit(1); 
	}else{
		media = mediaTest;
		controles[0] = posIni;
		controles[1] = regOc;
		controles[2] = accTot;
		controles[3] = fileSize;
	}
	/* codigo de debug*/
	//cout<<posIni<<endl<<regOc<<endl<<accTot<<endl<<fileSize<<endl<<mediaTest<<endl;
	
}

void printaConteudo(int posicao){
	reg seeker;
	fseek(arq, sizeof(reg)*posicao,SEEK_SET);
	fread(&seeker,sizeof(reg),1,arq);
	cout<<seeker.dado<<endl;
}

int main(){
	if(isEmpty()){
		initialize();
	}
	arq = fopen("database.bin","r+b");
	readHeader();
	char cmd;
	while(cin>>cmd, cmd!='e'){
		if(cmd=='p')printFile();
		else if(cmd=='i'){
			int chave;
			string dado;
			cin>>chave;
			cin.ignore();
			getline(cin, dado);
			if(!insereReg(chave, dado))cout<<"chave ja existente: "<<chave<<endl;
		}
		else if(cmd=='c'){
			int chave;
			cin>>chave;
			pair<int,bool> posicao = buscaChave(chave);
			if(posicao.second){
				cout<<"chave: "<<chave<<endl;
				printaConteudo(posicao.first);
			}
			else{
				cout<<"chave nao encontrada: "<<chave<<endl;
			}
		}
		else if(cmd=='r'){
			int chave;
			cin>>chave;
			if(!removeReg(chave)){
				cout<<"chave nao encontrada: "<<chave<<endl;
			}
		}
		else if(cmd=='m')cout<<fixed<<setprecision(1)<<media<<endl;
	}
	writeHeader();
	fclose(arq);
}
