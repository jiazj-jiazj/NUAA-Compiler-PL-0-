/*
	Created by 021830122��־�� on 2020/11/29
*/
#include"head.h"
fstream intxt;   //�ʷ����������ı�
fstream outtxt;  //�ʷ��ִ� ��������ı�
fstream readLexi; 
fstream writeGram;

#define LIT 0 
#define OPR 1
#define LOD 2
#define STO 3
#define CAL 4
#define INT 5
#define JMP 6
#define JPC 7
#define RED 8
#define WRT 9

#define constant 0
#define variable 1
#define procedure 2

string order[] = { "LIT","OPR","LOD","STO","CAL","INT","JMP","JPC","RED","WRT" };
const string key[] = { "program","const","var","procedure","begin","end","if","then","while","do","call","read","write","odd" };
int line;   //���뵥��������
int column;   //���뵥��������
Unit  unit;   //ÿ�����ʵĵ�Ԫ

int tx = 0;    //   ��ǰtable  ���ű��ַ��1��ʼ
int dx = 0;      //
int cx = 0;     //ָ����һ����ַ
int lev = 0;
int mm;
int err;
int isOutSym;
int isOutPCode;
int T;      //ջ���Ĵ���
int B;     //ջ��ַ�Ĵ���
int P;     //����ָ��
int I;    //ָ��Ĵ���
int dataStack[1000]; //����ջ

int ss=0;

struct SymTable {  //���ű�
	string name;
	int kind;
	int value;
	int level;
	int adr;
	int size;
	int num;    //�����ڳ�����ֵĴ����������ڴ��Ż�
}SymTable[1000];

struct Pcode {  //P����
	int f;
	int l;
	int a;
}Pcode[1000];

//Lexical analysis
void LA();     //�ʷ��������� 
bool isBC(char cc);   //�ʷ������ж��Ƿ���Ҫ����
bool isDigit(char cc);   //�ж��Ƿ�������
bool isLetter(char cc);   //�ж��Ƿ�����ĸ
string concat(char cc, string loken); //�������ַ����ӵ�����
int reserve(string loken);   //�ж����������Ƿ��Ǳ�ʶ��
void Retract();     //�����ַ�����һ��ָ��
//Grammar analysis
void GA();
void readLine();  //������һ������
void throwError(string error, int    ismissing); //������
void prog();
void block();
void condecl();
void const_();
void vardecl();
void proc();
void body();
void statement();
void lexp();
void exp();
void term();
void factor();

void entervar(string name, int level, int adr);    //���������뵽���ű�
void enterconst(string name, int level, int val);   //���������뵽���ű�
void enterprocedure(string name, int level, int adr);  //�����̵��뵽���ű�
int position(string name);      //�ڷ��ű��������Ϊname�ķ���
void gen(int f, int l, int a);   //����p����
int findproc();
int stringtoint(string name);   //�ַ�ת��Ϊ����
bool isNowExistTable(string name, int lev);      //�ж��Ƿ���ͬ��
bool isPreExistTable(string name, int lev);     //�ж��Ƿ���ͬ�������һ��
void interpreter();    //������
int getBase(int nowBp, int lev); //���ݲ���ҵ���ַ
void output();   //����м����
void outputtable();  //������ű�
void error(int n);  //��Ҫ���������������

void error(int n)
{

	switch (n)
	{
	case  -1:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing var/const" << endl; break;
	case  0:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  ;" << endl; break;
	case  1:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Identifier illegal" << endl; break;
	case  2:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  program" << endl; break;
	case  3:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  :=" << endl; break;
	case  4:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  (" << endl; break;
	case  6:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  Begin" << endl; break;
	case  7:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  End" << endl; break;
	case  8:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  Then" << endl; break;
	case  9:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing do" << endl; break;
	case 10:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << ":" << "Not exist " << unit.value << endl; break;
	case 11:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << unit.value << " is not a PROCEDURE" << endl; break;
	case 12:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << unit.value << " is not a VARIABLE" << endl; break;
	case 13:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << unit.value << " is not a VARIABLE" << endl; break;
	case 14:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "Not exist " << unit.value << endl; break;
	case 15:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "Duplicate definition  " << unit.value << endl; break;
	case 16:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "The number of parameters does not match" << endl; break;
	case 17:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "ILLEGAL" << endl; break;
	case 18:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "Missing Var" << endl; break;
	}

}
void outputtable()
{
	int i = 1;
	cout << "���ƣ�" << " " << "���� " << "   �������ڵĲ��" << " " << "���ŵ���Ե�ַ" << " " << "���ų��ֵ�����"<<endl;
	while (SymTable[i].num)
	{
		cout <<"���ƣ�  "<<SymTable[i].name << "   ����:   "  << SymTable[i].kind << "  �������ڵĲ��   "  << SymTable[i].level << "   ���ŵ���Ե�ַ   "  << SymTable[i].adr << "   ���ų��ֵ�����    " << SymTable[i].num << endl;
		i++;
	}
	
}
void output()    //���Pcode
{ 
	for (int i = 0; i < cx; i++)
	{
		cout << order[Pcode[i].f] << " ";
		cout << Pcode[i].l << " ";
		cout << Pcode[i].a << " ";
		cout << endl;
	}
}

int getBase(int nowBp, int lev)    //���ݲ��õ�����ַ
{ 
	int oldBp = nowBp;
	while (lev > 0)//�����ڲ��ʱѰ�ҷǾֲ�����
	{
		oldBp = dataStack[oldBp + 1];//ֱ�����Ļ��¼�׵�ַ
		lev--;
	}
	return oldBp;
}
  
int position(string name) {   //����ڷ��ű��λ��
	for (int i = tx; i >= 0; i--)
	{
		if (SymTable[i].name == name && SymTable[i].level <= lev)
			return i;
	}
	for (int i = tx; i >= 0; i--)
	{
		if (SymTable[i].name == name)
			return i;
	}
	return -1;
}
int stringtoint(string name) {
	int num = 0;
	for (int i = 0; i < name.size(); i++)
	{
		num = num * 10 + (name[i] - '0');
	}
	return num;
}

bool isNowExistTable(string name, int lev) {    //�Ƿ���ͬ��

	for (int i = 1; i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level == lev)
		{
			return 1;
		}
	}
	return 0;
}
bool isPreExistTable(string name, int lev) {  //�Ƿ���ͬ�������һ��

	for (int i = 1; i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level <= lev)
			return 1;
	}
	return 0;

}

void gen(int f, int l, int a) {  //����PCODE

	Pcode[cx].f = f;
	Pcode[cx].l = l;
	Pcode[cx].a = a;
	cx++;
}
int findproc()
{
	for (int i = tx; i >= 1; i--)
	{
		if (SymTable[i].kind == 2)
		{
			return i;
		}
	}
	return -1;
}

int main()
{

	LA();
	GA();

	return  0;
}

void interpreter()    //������
{
	P = 0;//�����ַ�Ĵ���
	B = 0;//��ַ�Ĵ���
	T = 0;//ջ���Ĵ���
	int t;
	do
	{
		I = P;
		P++;
		switch (Pcode[I].f)//��ȡα������
		{
		case 0:		//LIT 0 a��ȡ����a��������ջջ��
			dataStack[T] = Pcode[I].a;
			T++;
			break;
		case 1:     //OPR 0 a��ִ�����㣬a��ʾִ��ĳ������
			switch (Pcode[I].a)
			{
			case 0:						//opr,0,0 ���ù��̽����󣬷��ص��õ㲢��ջ
				T = B;
				P = dataStack[B + 2];	//���ص�ַ
				B = dataStack[B];		//��̬��
				break;
			case 1:                 //opr 0,1ȡ��ָ��
				dataStack[T - 1] = -dataStack[T - 1];
				break;
			case 2:                 //opr 0,2��ӣ���ԭ��������Ԫ����ȥ�����������ջ��
				dataStack[T - 2] = dataStack[T - 1] + dataStack[T - 2];
				T--;
				break;
			case 3:					//OPR 0,3 ��ջ����ȥջ����������ջԪ�أ����ֵ��ջ
				dataStack[T - 2] = dataStack[T - 2] - dataStack[T - 1];
				T--;
				break;
			case 4:    				//OPR 0,4��ջ������ջ����������ջԪ�أ����ֵ��ջ
				dataStack[T - 2] = dataStack[T - 1] * dataStack[T - 2];
				T--;
				break;
			case 5:					//OPR 0,5��ջ������ջ����������ջԪ�أ����ֵ��ջ
				dataStack[T - 2] = dataStack[T - 2] / dataStack[T - 1];
				T--;
				break;
			case 6:                 //ջ��Ԫ��ֵ��ż�жϣ����ֵ��ջ,����Ϊ1
				dataStack[T - 1] = dataStack[T - 1] % 2;
				break;
			case 7:
				break;
			case 8:					//��ջ����ջ���Ƿ���ȣ�������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 1] == dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 9:					//��ջ����ջ���Ƿ񲻵ȣ�������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 1] != dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 10:				//��ջ���Ƿ�С��ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] < dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 11:				//��ջ���Ƿ���ڵ���ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] >= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 12:				//��ջ���Ƿ����ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] > dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 13:				//��ջ���Ƿ�С�ڵ���ջ����������ջԪ�أ����ֵ��ջ
				if (dataStack[T - 2] <= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 15:				//��Ļ�������
				cout << endl;
				break;
			}
			break;
		case 2:  //LOD L ��a ȡ��������Ե�ַΪa�����ΪL���ŵ�����ջ��ջ��
			dataStack[T] = dataStack[Pcode[I].a + getBase(B, Pcode[I].l)];
			T++;
			break;
		case 3://STO L ��a ������ջջ�������ݴ����������Ե�ַΪa����β�ΪL��
			/*cout << "Pcode[I].a:" << Pcode[I].a << endl;
			cout << "B:" << B << endl;
			cout << "Pcode[I].l:" << Pcode[I].l << endl;
			cout << "getBase(B, Pcode[I].l):" << getBase(B, Pcode[I].l) << endl;
			cout << "Pcode[I].a + getBase(B, Pcode[I].l):" << Pcode[I].a + getBase(B, Pcode[I].l) << endl;
			cout << "T:" << T << endl;
			cout << "dataStack[T - 1]:" << dataStack[T - 1] << endl;*/
			dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = dataStack[T - 1];
			T--;
			break;
		case 4:		//CAL L ��a ���ù��̣�ת��ָ�����ڵ�ַΪa����β�ΪL��
			dataStack[T] = B;		//��̬����ֱ��������
			dataStack[T + 1] = getBase(B, Pcode[I].l);	//��̬��������ǰ���й���
			dataStack[T + 2] = P;		//���ص�ַ����һ��Ҫִ�е�
			B = T;
			P = Pcode[I].a;

			break;
		case 5:						//INT 0 ��a ����ջջ��ָ������a
			T = B + Pcode[I].a;
			break;
		case 6:						//JMP 0 ��a������ת�Ƶ���ַΪa��ָ��
			P = Pcode[I].a;
			break;
		case 7:						//JPC 0 ��a ����ת��ָ�ת�Ƶ���ַΪa��ָ��
			if (dataStack[T - 1] == 0)
			{
				P = Pcode[I].a;
			}
			break;
		case 8:					//RED L ��a �������ж���һ�����ݲ��������
			cin >> t;
			dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = t;
			break;
		case 9:					//ջ��ֵ�������Ļ
			cout << dataStack[T - 1];
			cout << "  ";
			break;
		}
	} while (P != 0);
}
void entervar(string name, int level, int adr)
{
	tx = tx + 1;
	SymTable[tx].kind = variable;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;
}
void enterconst(string name, int level, int val) {    //��¼���ű�

	tx = tx + 1;
	SymTable[tx].kind = constant;
	SymTable[tx].name = name;
	SymTable[tx].value = stringtoint(unit.value);
	SymTable[tx].level = level;
	SymTable[tx].value = val;
	SymTable[tx].num = 1;



}
void enterprocedure(string name, int level, int adr) {   //��¼���̷��ű�

	tx = tx + 1;
	SymTable[tx].kind = procedure;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;

}

void GA() //Grammar analysis
{
	readLexi.open("write.txt", ios::in);
	readLine();
	prog();


	int count = 0; //��¼δ��ʹ�õı�������
	int i = 1;
	while (SymTable[i].num && !SymTable[i].level)  //�ڷ��ű��б�����0���������ķ���
	{
		if (SymTable[i].num == 1 && SymTable[i].kind==variable) //����Ǳ�����ֻ�����һ�Σ�����û��ʹ�ã����к����Ż�
		{
			int j = i + 1;                                      
			while (SymTable[j].num && !SymTable[j].level)    //�������ж���ı�����Ե�ַ��Ҫ��һ
			{
				SymTable[j].adr--;                    //
				j++;
			}
			count++;    //δ��ʹ�õı���������һ
		}
		i++;
	}
	Pcode[mm].a -= count;     //����INTָ��ٵĿռ�����һ��ɨ��֮�󣬽�����֮ǰ

	/*cout << "�Ƿ����Pcode:1 or 0" << endl;
	cin >> isOutPCode;
	if (isOutPCode)
	{
		output();
	}
	cout << "�Ƿ�������ű�:1 or 0" << endl;
	cin >> isOutSym;
	if (isOutSym)
	{
		outputtable();
		cout << endl;
	}*/
	string line;
	if (!readLexi.eof())   //if prog is over ,but words still exit
	{
		throwError(unit.value, 0);
	}
	else
	{
		//interpreter();

		//exit(0);
	}
	while (!readLexi.eof())
	{
		getline(readLexi, line);
		if (readLexi.eof())
		{
			cout << "�Ƿ����Pcode:1 or 0" << endl;
			cin >> isOutPCode;
			if (isOutPCode)
			{
				output();
			}
			cout << "�Ƿ�������ű�:1 or 0" << endl;
			cin >> isOutSym;
			if (isOutSym)
			{
				outputtable();
				cout << endl;
			}
			interpreter();

			exit(0);
		}
		istringstream iss(line);
		iss >> unit.value;
		iss >> unit.key;
		iss >> unit.line;
		iss >> unit.column;
		throwError(unit.value, 0);
	}
	readLexi.close();
	cout << "�Ƿ����Pcode:1 or 0" << endl;
	cin >> isOutPCode;
	if (isOutPCode)
	{
		output();
	}
	cout << "�Ƿ�������ű�:1 or 0" << endl;
	cin >> isOutSym;
	if (isOutSym)
	{
		outputtable();
		cout << endl;
	}
	interpreter();

}
void LA()    //Lexical analysis
{
	char ch;
	string strToken;
	string nm;
	cout << "������Ҫ�򿪵��ļ���";
	cin >> nm;
	intxt.open(nm + ".txt", ios::in);
	outtxt.open("write.txt", ios::out | ios::trunc);
	while ((ch = intxt.get()) != -1)
	{
		if (!line)
			line++;
		if (ch == '/')
		{
			
			ch = intxt.get();
			if (ch == '*')
			{
				while (ch != '/')
				{
					ch = intxt.get();
					strToken = "";
				}
					
			}
			else
			{
				while (ch != ' ')
				{
					ch = intxt.get();
					strToken = "";
				}
				
			}
			if (isBC(ch))
			{
				strToken = "";
			}
			
			//Retract();
		}
		else {
			if (isBC(ch))
			{
				strToken = "";
			}
			else if (isLetter(ch))
			{
				while (isLetter(ch) || isDigit(ch))
				{
					strToken = concat(ch, strToken);
					column++;
					ch = intxt.get();
				}
				if (reserve(strToken))
				{
					cout << strToken << " " << "keyword" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << "keyword" << " " << line << " " << column << " " << endl;
				}
				else {
					cout << strToken << " " << "ID" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << "ID" << " " << line << " " << column << " " << endl;
				}
				Retract();
				strToken = "";
			}
			else if (isDigit(ch))
			{
				while (isDigit(ch))
				{
					strToken = concat(ch, strToken);
					column++;
					ch = intxt.get();
				}

				if (isLetter(ch))  //  char behaind int is wrong
				{
					cout << "[Lexical error]" << " " << line << " " << column << " " << endl;
					outtxt << "[Lexical error]" << " " << line << " " << column << " " << endl;

					while (isLetter(ch) || isDigit(ch))  //������ID
					{
						strToken = concat(ch, strToken);
						column++;
						ch = intxt.get();
					}
					cout << strToken << " " << " Invalid ID" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << " Invalid ID" << " " << line << " " << column << " " << endl;

					cout << "[Lexical error]" << " " << line << " " << column << " " << endl;
					outtxt << "[Lexical error]" << " " << line << " " << column << " " << endl;

				}
				else {
					cout << strToken << " " << " INT" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << " INT" << " " << line << " " << column << " " << endl;
				}
				Retract();
				strToken = "";

			}
			else switch (ch)
			{
			case '=':
				column++;
				cout << ch << " " << "lop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "lop" << " " << line << " " << column << " " << endl;
				break;
			case ':':
				column++;
				strToken = concat(ch, strToken);
				ch = intxt.get();

				strToken = concat(ch, strToken);
				if (ch == '=')
				{
					column++;
					cout << strToken << " " << "aop" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << "aop" << " " << line << " " << column << " " << endl;

				}
				else {
					cout << "Lexical error" << " " << "��=AOP" << line << " " << column << " " << "/.missing =./" << endl;
					outtxt << "Lexical error" << " " << "��=AOP" << line << " " << column << " " << "/.missing =./" << endl;
					Retract();
				}
				strToken = "";
				break;
			case '>':
				column++;
				ch = intxt.get();
				if (ch == '=')
				{
					column++;
					cout << ">=" << " " << "lop" << " " << line << " " << column << " " << endl;
					outtxt << ">=" << " " << "lop" << " " << line << " " << column << " " << endl;
				}
				else {
					cout << ">" << " " << "lop" << " " << line << " " << column << " " << endl;

					outtxt << ">" << " " << "lop" << " " << line << " " << column << " " << endl;
					Retract();
				}
				break;
			case '<':
				column++;
				ch = intxt.get();
				if (ch == '>')
				{
					column++;
					cout << "<>" << " " << "lop" << " " << line << " " << column << " " << endl;
					outtxt << "<>" << " " << "lop" << " " << line << " " << column << " " << endl;

				}
				else if (ch == '=')
				{
					column++;
					cout << "<=" << " " << "lop" << " " << line << " " << column << " " << endl;
					outtxt << "<=" << " " << "lop" << " " << line << " " << column << " " << endl;
				}
				else {
					cout << "<" << " " << "cop" << " " << line << " " << column << " " << endl;
					outtxt << "<" << " " << "cop" << " " << line << " " << column << " " << endl;
					Retract();
				}
				break;
			case '+':
				column++;
				cout << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				break;

			case '-':
				column++;
				cout << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				break;

			case '*':
				column++;
				cout << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				break;
			case '/':
				column++;
				cout << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				break;
			case ',':
				column++;
				cout << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				break;
			case ';':
				column++;
				outtxt << ch << " " << "eop" << " " << line << " " << column << " " << endl;
				cout << ch << " " << "eop" << " " << line << " " << column << " " << endl;
				break;
			case '(':

			case ')':
				column++;
				cout << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				break;



			default:
				column++;
				cout << ch << " " << "UNKNOW" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "UNKNOW" << " " << line << " " << column << " " << endl;
				break;
			}
		}


	}
	intxt.close();
	outtxt.close();
}


//Lexical
bool isBC(char cc)   //�ж϶����Ƿ���Ҫ��Ծ
{
	if (cc == '\n' || cc == '\t' || cc == ' ' || cc == '\r')
	{
		switch (cc)
		{
		case ' ':
			column++;
			break;
		case '\n':
			line++;
			column = 0;
			break;
		case '\t':
			column += 4;
			break;
		case '\r':
			column = 1;
		default:
			break;
		}
		return 1;
	}
	return 0;
}

bool isDigit(char cc)
{
	if (cc >= '0' && cc <= '9')
		return 1;
	return 0;
}

bool isLetter(char cc)   //�ж��Ƿ�����ĸ
{
	if ((cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z'))
	{
		return 1;

	}
	return 0;
}

string concat(char cc, string loken)  //�������ַ����ӵ�����
{
	return loken + cc;
}

int reserve(string loken)   //�ж��Ƿ��Ǳ�ʶ��
{
	int i;
	for (int i = 0; i < 15; i++)
	{
		if (loken == key[i])
			return 1;
	}
	return 0;
}


void Retract()   //���뵥�ʺ���һ��ָ��
{
	if (!intxt.eof())
		intxt.seekg(-1, ios::cur);

}

//Grammar  
void readLine()   //ÿ�δӴʷ���������һ������
{
	string line;
	if (!readLexi.eof())
	{
		getline(readLexi, line);
		if (readLexi.eof())
		{
			return;
		}
		istringstream iss(line);
		iss >> unit.value;
		iss >> unit.key;
		iss >> unit.line;
		iss >> unit.column;
	}
	else {
		return;
	}
}

void throwError(string error, int ismissing)   //����
{
	if (ismissing == 1)
	{
		cout << error + " is missing ";
		unit.print();
	}
	if (ismissing == 0)
	{
		cout << error + " is extra ";
		unit.print();
	}
}
void prog()    
{

	while (unit.value != "program")
	{
		throwError(unit.value, 0);
		readLine();
	}
	readLine();
	if (unit.key != "ID")
	{
		throwError("ID", 1);
	}
	else {
		readLine();
	}
	if (unit.value != ";")
	{
		throwError(";", 1);
	}
	else {
		readLine();
	}
	block();
}
void block()
{
	int dx0 = dx;
	int tx0 = tx + 1;
	int n = 0;

	if (tx0 > 1)
	{
		n = findproc();
		tx0 -= SymTable[n].size;//��ȥ�βθ���

	}

	if (tx0 > 1)
	{
		dx = 3 + SymTable[n].size;//
	}
	else {
		dx = 3;
	}


	int cx0 = cx;
	gen(JMP, 0, 0);


	while ((unit.value != "const") && (unit.value != "var") && (unit.value != "procedure") && (unit.value != "begin"))
	{
		throwError(unit.value, 0);
		readLine();
	}
	if (unit.value == "const")
	{
		condecl();
	}
	if (unit.value == "var")
	{
		vardecl();    //���ɷ��ű�������ָ��
	}
	if (unit.value == "procedure")
	{
		proc();
		lev--;
	}
	if (tx0 > 1)
	{
		int i;

		n = findproc();


		for (i = 0; i < SymTable[n].size; i++)     //������
		{
			gen(STO, 0, SymTable[n].size + 3 - 1 - i);
		}
	}

	Pcode[cx0].a = cx;

	mm = cx;

	gen(INT, 0, dx);

	if (tx != 1)
	{
		SymTable[n].value = cx - 1 - SymTable[n].size;  //������ڵ�ַ��   ������SymTable[n].size ��ָ��,�� ������
	}

	body();
	gen(OPR, 0, 0);

	tx = tx0;   // clear Sym
	dx = dx0;
	int count = 0;



}

void condecl()
{
	while (unit.value != "const")
	{
		throwError(unit.value, 0);
		readLine();
	}
	readLine();
	const_();
	while (unit.value == ",")
	{
		readLine();
		const_();
	}
	if (unit.value != ";")
	{
		throwError(";", 1);
	}
	else
	{
		readLine();
	}
}

void const_()
{
	while (unit.key != "ID")
	{
		throwError(unit.value, 0);
		readLine();
	}
	string name = unit.value;

	readLine();
	if (unit.value != ":=")
	{
		throwError(":=", 1);
	}
	else {
		readLine();
	}
	if (unit.key != "INT")
	{
		throwError("INT", 1);
	}
	else {
		int value = stringtoint(unit.value);

		if (isNowExistTable(name, lev))
		{
			error(15);
		}

		enterconst(name, lev, value);


		readLine();

	}
}
void vardecl()
{
	while (unit.value != "var")
	{
		throwError(unit.value, 0);
		readLine();
	}
	readLine();
	if (unit.key != "ID")
	{
		throwError("ID", 1);
	}
	else {
		string name = unit.value;
		if (isNowExistTable(name, lev))
		{
			error(15);
		}

		entervar(name, lev, dx);
		dx++;


		readLine();
	}
	while (unit.value == ",")
	{
		readLine();
		if (unit.key != "ID")
		{
			throwError("ID", 1);
		}
		else {
			string name = unit.value;

			if (isNowExistTable(name, lev))
			{
				error(15);
			}

			entervar(name, lev, dx);
			dx++;
			readLine();
		}

	}
	if (unit.value != ";")
	{
		throwError(";", 1);
	}
	else {
		readLine();
	}
}
void proc()
{
	int count = 0;
	int tx0;
	while (unit.value != "procedure")
	{
		throwError(unit.value, 0);
		readLine();
	}
	readLine();
	if (unit.key != "ID")
	{
		throwError("ID", 1);
	}
	else {
		string name = unit.value;
		if (isNowExistTable(name, lev))
		{
			error(15);
		}
		tx0 = tx + 1;
		enterprocedure(name, lev, dx);
		lev++;

		readLine();
	}
	if (unit.value != "(")
	{
		throwError("(", 1);
	}
	else {
		readLine();
	}
	if (unit.key == "ID")
	{
		string name = unit.value;
		entervar(name, lev, 3 + count);
		count++;
		SymTable[tx0].size = count;

		readLine();
		while (unit.value == ",")
		{
			readLine();
			if (unit.key != "ID")
			{
				throwError("ID", 1);
			}
			else {
				string name = unit.value;
				entervar(name, lev, 3 + count);
				count++;
				SymTable[tx0].size = count;
				readLine();
			}
		}
	}
	if (unit.value != ")")
	{
		throwError(")", 1);
	}
	else {
		readLine();
	}
	if (unit.value != ";")
	{
		throwError(";", 1);
	}
	else {
		readLine();
	}
	block();
	while (unit.value == ";")
	{
		readLine();
		proc();
	}
}
void body()
{
	while (unit.value != "begin")
	{
		throwError(unit.value, 0);
		readLine();
	}
	readLine();
	statement();
	while (unit.value == ";")
	{
		readLine();
		statement();
	}
	if (unit.value != "end")
	{
		throwError("end", 1);
	}
	else {
		readLine();
	}
}
void statement()
{
	while (unit.key != "ID"&&unit.value != "if"&&unit.value != "while"&&unit.value != "call"&&unit.value != "begin"&&unit.value != "read"&&unit.value != "write")
	{
		throwError(unit.value, 0);
		readLine();
	}
	if (unit.key == "ID")
	{
		string name = unit.value;
		readLine();
		if (unit.value != ":=")
		{
			throwError(":=", 1);
		}
		else {
			readLine();
		}
		int i = position(name);
		SymTable[i].num++;
		exp();
		/*	cout << SymTable[i].level<<endl;
			cout << lev << endl;*/


		if (!isPreExistTable(name, lev))
		{
			error(14);
		}
		else {
			//int i = position(name);
	/*		cout << "SymTable[i].name:"<<SymTable[i].name << endl;
			cout << "lev:" << lev << endl;*/

			if (SymTable[i].kind == variable)
			{
				gen(STO, lev - SymTable[i].level, SymTable[i].adr);
			}
			else {
				error(13);
			}
		}
	}
	else if (unit.value == "if")
	{
		readLine();
		lexp();
		int cx1;
		if (unit.value != "then")
		{
			throwError("then", 1);
		}
		else {
			cx1 = cx;
			gen(JPC, 0, 0);
			readLine();
		}
		statement();
		int cx2 = cx;
		gen(JMP, 0, 0);

		Pcode[cx1].a = cx;
		Pcode[cx2].a = cx;

		if (unit.value == "else")
		{
			readLine();
			statement();
			Pcode[cx2].a = cx;
		}
	}
	else if (unit.value == "while")
	{
		int cx2;
		int cx1 = cx;
		readLine();
		lexp();
		//if (unit.value != "do")
		//{
		//	throwError("do", 1);
		//}
		//else {
		cx2 = cx;
		gen(JPC, 0, 0);

		readLine();
		//}
		statement();
		gen(JMP, 0, cx1);
		Pcode[cx2].a = cx;

	}
	else if (unit.value == "call")
	{
		readLine();
		int count = 0, i;
		if (unit.key != "ID")
		{
			throwError("ID", 1);
		}
		else
		{
			string name = unit.value;
			if (!isPreExistTable(name, lev))
			{
				i = position(name);
				SymTable[i].num++;
				error(10);

			}
			else {
				i = position(name);
				SymTable[i].num++;

				if (SymTable[i].kind == 2)
				{

				}
				else {
					error(11);

				}
			}
			readLine();
		}
		if (unit.value != "(")
		{
			throwError("(", 1);
		}
		else {
			readLine();
		}
		if (unit.value == ")")
		{
			gen(CAL, lev - SymTable[i].level, SymTable[i].value);   //

		}
		if (unit.value == "+" || unit.value == "-" || unit.key == "ID" || unit.key == "INT" || unit.value == "(")
		{
			exp();
			count++;
			while (unit.value == ",")
			{
				readLine();
				exp();
				count++;
			}
			if (count != SymTable[i].size)
			{
				error(16);

			}
			gen(CAL, lev - SymTable[i].level, SymTable[i].value);  //SymTable[i].value Ҫ��ת�Ĺ���ָ���ַ

		}
		if (unit.value != ")")
		{
			throwError(")", 1);
		}
		else {
			readLine();
		}
	}
	else if (unit.value == "begin")
	{
		body();
	}
	else if (unit.value == "read")
	{
		readLine();
		if (unit.value != "(")
		{
			throwError("(", 1);
		}
		else {
			readLine();
		}
		if (unit.key != "ID")
		{
			throwError("ID", 1);
		}
		else {
			string name = unit.value;
			if (!isPreExistTable(name, lev))
			{
				error(10);
			}
			else {
				int i = position(name);
				SymTable[i].num++;

				if (SymTable[i].kind == 1)
				{
					gen(RED, lev - SymTable[i].level, SymTable[i].adr);
				}
				else {
					error(12);
				}
			}
			readLine();
		}
		while (unit.value == ",")
		{
			readLine();
			string name = unit.value;
			if (!isPreExistTable(name, lev))
			{
				error(10);
			}
			else {
				int i = position(name);
				SymTable[i].num++;

				if (SymTable[i].kind == 1)
				{
					gen(RED, lev - SymTable[i].level, SymTable[i].adr);
				}
				else {
					error(12);
				}
			}
			readLine();
		}
		if (unit.value != ")")
		{
			throwError(")", 1);
		}
		else {
			readLine();
		}
	}
	else if (unit.value == "write")
	{
		readLine();
		if (unit.value != "(")
		{
			throwError("(", 1);
		}
		else readLine();
		exp();
		gen(WRT, 0, 0);
		while (unit.value == ",")
		{
			readLine();
			exp();
			gen(WRT, 0, 0);
		}
		gen(OPR, 0, 15);
		if (unit.value != ")")
		{
			throwError(")", 1);
		}
		else {
			readLine();
		}
	}
}
int lop()
{
	if (unit.value == "=")
	{
		readLine();
		return 0;
	}
	else if (unit.value == "<>")
	{
		readLine();
		return 1;
	}
	else if (unit.value == "<")
	{
		readLine();
		return 2;
	}
	else if (unit.value == "<=")
	{
		readLine();
		return 3;
	}
	else if (unit.value == ">")
	{
		readLine();
		return 4;
	}
	else if (unit.value == ">=")
	{
		readLine();
		return 5;
	}
	return -1;
}
void lexp() {
	if (unit.value == "odd")
	{
		readLine();
		exp();
		gen(OPR, 0, 6);
	}
	else {
		exp();
		int i = lop();
		exp();
		if (i == 0)
		{
			gen(OPR, 0, 8);
		}
		else if (i == 1)
		{
			gen(OPR, 0, 9);
		}
		else if (i == 2)
		{
			gen(OPR, 0, 10);
		}
		else if (i == 3)
		{
			gen(OPR, 0, 13);
		}
		else if (i == 4)
		{
			gen(OPR, 0, 12);
		}
		else if (i == 5)
		{
			gen(OPR, 0, 11);
		}
	}
}
void exp()
{
	string temp;
	while (unit.value != "+"&&unit.value != "-"&&unit.key != "ID"&&unit.key != "INT"&&unit.value != "(")
	{
		throwError(unit.value, 0);
		readLine();
	}
	if (unit.value == "+" || unit.value == "-")
	{
		temp = unit.value;

		readLine();
	}
	term();
	if (temp == "-")
	{
		gen(OPR, 0, 1);
	}

	while (unit.key == "aop")
	{
		temp = unit.value;
		readLine();
		term();
		if (temp == "+")
		{
			gen(OPR, 0, 2);
		}
		else {
			gen(OPR, 0, 3);
		}
	}
}
void term()
{
	factor();
	while (unit.key == "mop")
	{
		string temp = unit.key;

		readLine();
		factor();

		if (temp == "*")
		{
			gen(OPR, 0, 4);
		}
		else if (temp == "/")
		{
			gen(OPR, 0, 5);
		}
	}
}
void factor()
{
	int i;
	while (unit.key != "ID"&&unit.key != "INT"&&unit.value != "(")
	{
		throwError(unit.value, 0);
		readLine();
	}
	if (unit.key == "ID")
	{
		string name = unit.value;

		if (!isPreExistTable(name, lev))
		{
			error(10);
		}
		else {
			i = position(name);
							SymTable[i].num++;

			if (SymTable[i].kind == 1)
			{
				gen(LOD, lev - SymTable[i].level, SymTable[i].adr);
			}
			else if (SymTable[i].kind == 0)
			{
				gen(LIT, 0, SymTable[i].value);
			}
			else
			{
				error(12);
				return;
			}
		}
		readLine();
	}
	else if (unit.key == "INT")
	{
		gen(LIT, 0, stringtoint(unit.value));
		readLine();
	}
	else if (unit.value == "(")
	{
		readLine();
		exp();
		if (unit.value != ")")
		{
			throwError(")", 1);
		}
		else {
			readLine();
		}
	}
}
