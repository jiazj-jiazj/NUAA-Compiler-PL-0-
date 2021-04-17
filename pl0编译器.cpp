/*
	Created by 021830122贾志军 on 2020/11/29
*/
#include"head.h"
fstream intxt;   //词法分析读入文本
fstream outtxt;  //词法分词 单词输出文本
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
int line;   //读入单词所在行
int column;   //读入单词所在列
Unit  unit;   //每个单词的单元

int tx = 0;    //   当前table  符号表地址从1开始
int dx = 0;      //
int cx = 0;     //指令下一个地址
int lev = 0;
int mm;
int err;
int isOutSym;
int isOutPCode;
int T;      //栈顶寄存器
int B;     //栈基址寄存器
int P;     //下条指令
int I;    //指令寄存器
int dataStack[1000]; //数据栈

int ss=0;

struct SymTable {  //符号表
	string name;
	int kind;
	int value;
	int level;
	int adr;
	int size;
	int num;    //变量在程序出现的次数，方便内存优化
}SymTable[1000];

struct Pcode {  //P代码
	int f;
	int l;
	int a;
}Pcode[1000];

//Lexical analysis
void LA();     //词法分析函数 
bool isBC(char cc);   //词法分析判断是否需要跳读
bool isDigit(char cc);   //判断是否是数字
bool isLetter(char cc);   //判断是否是字母
string concat(char cc, string loken); //将读入字符连接到单词
int reserve(string loken);   //判断所读单词是否是标识符
void Retract();     //读入字符后退一个指针
//Grammar analysis
void GA();
void readLine();  //读入下一个单词
void throwError(string error, int    ismissing); //错误处理
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

void entervar(string name, int level, int adr);    //将变量登入到符号表
void enterconst(string name, int level, int val);   //将常量登入到符号表
void enterprocedure(string name, int level, int adr);  //将过程登入到符号表
int position(string name);      //在符号表查找名字为name的符号
void gen(int f, int l, int a);   //产生p代码
int findproc();
int stringtoint(string name);   //字符转换为整数
bool isNowExistTable(string name, int lev);      //判断是否是同层
bool isPreExistTable(string name, int lev);     //判断是否是同层或者上一层
void interpreter();    //解释器
int getBase(int nowBp, int lev); //根据层差找到基址
void output();   //输出中间代码
void outputtable();  //输出符号表
void error(int n);  //主要是语义分析错误处理

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
	cout << "名称：" << " " << "类型 " << "   符号所在的层次" << " " << "符号的相对地址" << " " << "符号出现的数量"<<endl;
	while (SymTable[i].num)
	{
		cout <<"名称：  "<<SymTable[i].name << "   类型:   "  << SymTable[i].kind << "  符号所在的层次   "  << SymTable[i].level << "   符号的相对地址   "  << SymTable[i].adr << "   符号出现的数量    " << SymTable[i].num << endl;
		i++;
	}
	
}
void output()    //输出Pcode
{ 
	for (int i = 0; i < cx; i++)
	{
		cout << order[Pcode[i].f] << " ";
		cout << Pcode[i].l << " ";
		cout << Pcode[i].a << " ";
		cout << endl;
	}
}

int getBase(int nowBp, int lev)    //根据层差得到基地址
{ 
	int oldBp = nowBp;
	while (lev > 0)//当存在层差时寻找非局部变量
	{
		oldBp = dataStack[oldBp + 1];//直接外层的活动记录首地址
		lev--;
	}
	return oldBp;
}
  
int position(string name) {   //输出在符号表的位置
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

bool isNowExistTable(string name, int lev) {    //是否在同层

	for (int i = 1; i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level == lev)
		{
			return 1;
		}
	}
	return 0;
}
bool isPreExistTable(string name, int lev) {  //是否在同层或者上一层

	for (int i = 1; i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level <= lev)
			return 1;
	}
	return 0;

}

void gen(int f, int l, int a) {  //产生PCODE

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

void interpreter()    //解释器
{
	P = 0;//程序地址寄存器
	B = 0;//基址寄存器
	T = 0;//栈顶寄存器
	int t;
	do
	{
		I = P;
		P++;
		switch (Pcode[I].f)//获取伪操作码
		{
		case 0:		//LIT 0 a，取常量a放入数据栈栈顶
			dataStack[T] = Pcode[I].a;
			T++;
			break;
		case 1:     //OPR 0 a，执行运算，a表示执行某种运算
			switch (Pcode[I].a)
			{
			case 0:						//opr,0,0 调用过程结束后，返回调用点并退栈
				T = B;
				P = dataStack[B + 2];	//返回地址
				B = dataStack[B];		//静态链
				break;
			case 1:                 //opr 0,1取反指令
				dataStack[T - 1] = -dataStack[T - 1];
				break;
			case 2:                 //opr 0,2相加，将原来的两个元素退去，将结果置于栈顶
				dataStack[T - 2] = dataStack[T - 1] + dataStack[T - 2];
				T--;
				break;
			case 3:					//OPR 0,3 次栈顶减去栈顶，退两个栈元素，结果值进栈
				dataStack[T - 2] = dataStack[T - 2] - dataStack[T - 1];
				T--;
				break;
			case 4:    				//OPR 0,4次栈顶乘以栈顶，退两个栈元素，结果值进栈
				dataStack[T - 2] = dataStack[T - 1] * dataStack[T - 2];
				T--;
				break;
			case 5:					//OPR 0,5次栈顶除以栈顶，退两个栈元素，结果值进栈
				dataStack[T - 2] = dataStack[T - 2] / dataStack[T - 1];
				T--;
				break;
			case 6:                 //栈顶元素值奇偶判断，结果值进栈,奇数为1
				dataStack[T - 1] = dataStack[T - 1] % 2;
				break;
			case 7:
				break;
			case 8:					//次栈顶与栈项是否相等，退两个栈元素，结果值进栈
				if (dataStack[T - 1] == dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 9:					//次栈顶与栈项是否不等，退两个栈元素，结果值进栈
				if (dataStack[T - 1] != dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 10:				//次栈顶是否小于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] < dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 11:				//次栈顶是否大于等于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] >= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 12:				//次栈顶是否大于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] > dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 13:				//次栈顶是否小于等于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] <= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
					break;
				}
				dataStack[T - 2] = 0;
				T--;
				break;
			case 15:				//屏幕输出换行
				cout << endl;
				break;
			}
			break;
		case 2:  //LOD L ，a 取变量（相对地址为a，层差为L）放到数据栈的栈顶
			dataStack[T] = dataStack[Pcode[I].a + getBase(B, Pcode[I].l)];
			T++;
			break;
		case 3://STO L ，a 将数据栈栈顶的内容存入变量（相对地址为a，层次差为L）
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
		case 4:		//CAL L ，a 调用过程（转子指令）（入口地址为a，层次差为L）
			dataStack[T] = B;		//静态链，直接外层过程
			dataStack[T + 1] = getBase(B, Pcode[I].l);	//动态链，调用前运行过程
			dataStack[T + 2] = P;		//返回地址，下一条要执行的
			B = T;
			P = Pcode[I].a;

			break;
		case 5:						//INT 0 ，a 数据栈栈顶指针增加a
			T = B + Pcode[I].a;
			break;
		case 6:						//JMP 0 ，a无条件转移到地址为a的指令
			P = Pcode[I].a;
			break;
		case 7:						//JPC 0 ，a 条件转移指令，转移到地址为a的指令
			if (dataStack[T - 1] == 0)
			{
				P = Pcode[I].a;
			}
			break;
		case 8:					//RED L ，a 从命令行读入一个数据并存入变量
			cin >> t;
			dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = t;
			break;
		case 9:					//栈顶值输出至屏幕
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
void enterconst(string name, int level, int val) {    //登录符号表

	tx = tx + 1;
	SymTable[tx].kind = constant;
	SymTable[tx].name = name;
	SymTable[tx].value = stringtoint(unit.value);
	SymTable[tx].level = level;
	SymTable[tx].value = val;
	SymTable[tx].num = 1;



}
void enterprocedure(string name, int level, int adr) {   //登录进程符号表

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


	int count = 0; //记录未被使用的变量个数
	int i = 1;
	while (SymTable[i].num && !SymTable[i].level)  //在符号表中遍历第0层主函数的符号
	{
		if (SymTable[i].num == 1 && SymTable[i].kind==variable) //如果是变量且只定义过一次，后续没有使用，进行后续优化
		{
			int j = i + 1;                                      
			while (SymTable[j].num && !SymTable[j].level)    //后续所有定义的变量相对地址都要减一
			{
				SymTable[j].adr--;                    //
				j++;
			}
			count++;    //未被使用的变量个数加一
		}
		i++;
	}
	Pcode[mm].a -= count;     //回填INT指令开辟的空间数，一边扫描之后，解释器之前

	/*cout << "是否输出Pcode:1 or 0" << endl;
	cin >> isOutPCode;
	if (isOutPCode)
	{
		output();
	}
	cout << "是否输出符号表:1 or 0" << endl;
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
			cout << "是否输出Pcode:1 or 0" << endl;
			cin >> isOutPCode;
			if (isOutPCode)
			{
				output();
			}
			cout << "是否输出符号表:1 or 0" << endl;
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
	cout << "是否输出Pcode:1 or 0" << endl;
	cin >> isOutPCode;
	if (isOutPCode)
	{
		output();
	}
	cout << "是否输出符号表:1 or 0" << endl;
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
	cout << "请输入要打开的文件名";
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

					while (isLetter(ch) || isDigit(ch))  //继续读ID
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
					cout << "Lexical error" << " " << "：=AOP" << line << " " << column << " " << "/.missing =./" << endl;
					outtxt << "Lexical error" << " " << "：=AOP" << line << " " << column << " " << "/.missing =./" << endl;
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
bool isBC(char cc)   //判断读入是否需要跳跃
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

bool isLetter(char cc)   //判断是否是字母
{
	if ((cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z'))
	{
		return 1;

	}
	return 0;
}

string concat(char cc, string loken)  //将读入字符连接到单词
{
	return loken + cc;
}

int reserve(string loken)   //判断是否是标识符
{
	int i;
	for (int i = 0; i < 15; i++)
	{
		if (loken == key[i])
			return 1;
	}
	return 0;
}


void Retract()   //读入单词后退一个指针
{
	if (!intxt.eof())
		intxt.seekg(-1, ios::cur);

}

//Grammar  
void readLine()   //每次从词法分析读入一个单词
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

void throwError(string error, int ismissing)   //报错
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
		tx0 -= SymTable[n].size;//减去形参个数

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
		vardecl();    //生成符号表，不生成指令
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


		for (i = 0; i < SymTable[n].size; i++)     //在这里
		{
			gen(STO, 0, SymTable[n].size + 3 - 1 - i);
		}
	}

	Pcode[cx0].a = cx;

	mm = cx;

	gen(INT, 0, dx);

	if (tx != 1)
	{
		SymTable[n].value = cx - 1 - SymTable[n].size;  //过程入口地址，   会生成SymTable[n].size 个指令,见 在这里
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
			gen(CAL, lev - SymTable[i].level, SymTable[i].value);  //SymTable[i].value 要跳转的过程指令地址

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
