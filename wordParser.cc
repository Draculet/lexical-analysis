#include <iostream>
#include <map>
#include <vector>
#include <stdio.h>
#include "Buffer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

using namespace std;
using namespace buf;

struct node//二元组
{
    node(int cla, string s):
        classid(cla),
        sval(s)
    {

    }

    node(int cla, double d):
        classid(cla),
        dval(d)
    {

    }

    node(int cla, int i):
        classid(cla),
        ival(i)
    {

    }

    void print()
    {
        if (sval.size() != 0)
        {
            printf("[%4d %4s]\n", classid, sval.c_str());
        }
        else if (dval != -9999)
        {
            printf("[%4d %4f]\n", classid, dval);
        }
        else if (ival != -9999)
        {
            printf("[%4d %4d]\n", classid, ival);
        }
    }
    int classid;//代表无效值
    string sval;
    double dval = -9999;//代表无效值
    int ival = -9999;
};

class wordParser
{
    public:
    wordParser()
    {
        types_["void"] = 1;
        types_["int"] = 2;
        types_["double"] = 3;
        types_["float"] = 4;
        types_["main"] = 5;
        types_["return"] = 6;
        types_["variable"] = 7;//变量
        types_["constant"] = 8;//常数
        types_["("] = 9;
        types_[")"] = 10;
        types_["{"] = 11;
        types_["}"] = 12;
        types_[","] = 13;
        types_["."] = 14;
        types_[";"] = 15;
        types_["["] = 16;
        types_["]"] = 17;
        types_["~"] = 18;
        //types_["\'"] = 19;
        //types_["\""] = 20;
        types_["char"] = 21;
        types_["long"] = 22;
        types_["auto"] = 23;
        types_["break"] = 24;
        types_["case"] = 25;
        types_["switch"] = 26;
        types_["const"] = 27;
        types_["continue"] = 28;
        types_["else"] = 29;
        types_["for"] = 30;
        types_["if"] = 31;
        types_["short"] = 32;
        types_["unsigned"] = 33;
        types_["struct"] = 34;
        types_["class"] = 35;
        types_["while"] = 36;
        types_["<="] = 37;
        types_["<"] = 38;
        types_["<<"] = 39;
        types_[">"] = 40;
        types_[">="] = 41;
        types_[">>"] = 42;
        types_["="] = 43;
        types_["=="] = 44;
        types_["!"] = 45;
        types_["!="] = 46;
        types_["/"] = 47;
        types_["/="] = 48;
        types_["*"] = 49;
        types_["*="] = 50;
        types_["-"] = 51;
        types_["-="] = 52;
        types_["--"] = 53;
        types_["+"] = 54;
        types_["+="] = 55;
        types_["++"] = 56;
        types_["union"] = 57;
    }
    void upload();
    int pretreat();
    void parse();

    void print()
    {
        for (auto iter : tuples_)
        {
            iter.print();
        }
    }

    void printClass()
    {
        printf("\n关键字表:\n");
        for (auto t : types_)
        {
            printf("%4d%15s\n", t.second, t.first.c_str());
        }
        printf("\n\n");
    }

    void printInt()
    {
        printf("\n常数表(整型):\n");
        int index = 1;
        for (auto t : ints_)
        {
            printf("%4d%15d\n", index, t);
            index++;
        }
        printf("\n\n");
    }

    void printStr()
    {
        printf("\n常数表(字符型):\n");
        int index = 1;
        for (auto t : conststrs_)
        {
            printf("%4d%15s\n", index, t.c_str());
            index++;
        }
        printf("\n\n");
    }

    void printVariable()
    {
        printf("\n变量表:\n");
        int index = 1;
        for (auto t : variables_)
        {
            printf("%4d%15s\n", index, t.c_str());
            index++;
        }
        printf("\n\n");
    } 

    void printDouble()
    {
        printf("\n常数表(浮点型):\n");
        int index = 1;
        for (auto t : doubles_)
        {
            printf("%4d%15f\n", index, t);
            index++;
        }
        printf("\n\n");
    }
    Buffer buf;

    private:
    map<string, string> defines_;
    vector<string> paths_;
    vector<string> variables_;//变量名登记表
    vector<int> ints_;//常数登记表
    vector<double> doubles_;//常数登记表
    vector<string> conststrs_;//字面值常量登记表
    vector<node> tuples_;//存储二元组
    map<string, int> types_;
};

void wordParser::upload()
{
    int ffd = open("ex.txt", O_RDONLY);
    if (ffd == -1)
    {
        printf("Error Open\n");
        exit(-1);
    }
    else
    {
        char cbuf[1024 * 10] = {0};
        int ret = read(ffd, cbuf, sizeof(cbuf));
        buf.append(cbuf, ret);
        close(ffd);
    }
}

int wordParser::pretreat()
{
    char fbuf[1024 * 1024] = {0};
    char *p = fbuf;
    int ffd = open("./example/ex.cc", O_RDONLY);
    if (ffd == -1)
    {
        printf("Error Open\n");
        exit(-1);
    }
    else
    {
        char cbuf[1024 * 10] = {0};
        int ret = read(ffd, cbuf, sizeof(cbuf));
        close(ffd);
        int pos = 0;
        int pos2 = 0;
        int cur = 0;
        string str(cbuf, ret);
        while (1)
        {
            pos = str.find("#include \"", 0);
            if (pos == string::npos)
                break;
            pos2 = str.find("\n", pos);
            string path(str, pos + 10, pos2 - pos - 11);
            paths_.push_back(path);
            //cout << path << endl;
            str = string(str, pos2 + 1);
        }
        //cout << str << endl;
        for (auto path : paths_)
        {
            //cout << path << endl;
            int ffd = open(path.c_str(), O_RDONLY);
            if (ffd == -1)
            {
                printf("Error Open\n");
                close(ffd);
                exit(-1);
            }
            else
            {
                int ret = read(ffd, p, sizeof(fbuf));
                p += ret;
                close(ffd);
            }
        }
        pos = 0;
        pos2 = 0;
        cur = 0;
        int pos3 = 0;
        while (1)
        {
            pos = str.find("#define", cur);
            if (pos == string::npos)
                break;
            pos2 = str.find(" ", pos + 8);
            pos3 = str.find("\n", pos2 + 1);
            string key(str, pos + 8, pos2 - pos - 8);
            string value(str, pos2 + 1, pos3 - pos2 - 1);
            //cout << key << endl;
            //cout << value << endl;
            defines_[key] = value;
            str = string(str, pos3 + 1);
        }
        //cout << str << endl;
        //define替换
        for (auto k : defines_)
        {
            while (1)
            {
                pos = str.find(k.first, cur);
                if (pos == string::npos)
                    break;
                string before(str, 0, pos);
                //cout << before << endl;
                string after(str, pos + k.first.size());
                string s = before + k.second + after;
                str = s;
                //cout << after << endl;
                //pos3 = s.find("\n", pos2 + 1);
                //string key(s, pos + 8, pos2 - pos - 8);
                //string value(s, pos2 + 1, pos3 - pos2 - 1);
                //cout << key << endl;
                //cout << value << endl;
                //cout << path << endl;
                //str = string(s, pos3 + 1);
                cur = cur + pos + k.first.size();
            }
            cur = 0;
        }
        //cout << str << endl;
        memcpy(p, str.c_str(), str.size());
        printf("预处理后的代码:\n%s\n", fbuf);
        buf.append(fbuf, strlen(fbuf));
    }
}

void wordParser::parse()
{
    cout << buf.preViewAsString(buf.readable()) << endl;
    string str;
    char ch;
    int state = 0;
    vector<char> word;
    while(1)
    {
        if (buf.readable() == 0)
            break;
        switch(state)
        {
            case 0:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
                {
                    state = 0;
                }
                else if (isdigit(ch))
                {
                    word.push_back(ch);
                    state = 3;
                }
                else if (isalpha(ch))
                {
                    word.push_back(ch);
                    state = 1;
                }
                else if (ch == '<')
                {
                    word.push_back(ch);
                    state = 8;
                }
                else if (ch == '>')
                {
                    word.push_back(ch);
                    state = 12;
                }
                else if (ch == '=')
                {
                    word.push_back(ch);
                    state = 16;
                }
                else if (ch == '!')
                {
                    word.push_back(ch);
                    state = 33;
                }
                else if (ch == '/')
                {
                    word.push_back(ch);
                    state = 30;
                }
                else if (ch == '+')
                {
                    word.push_back(ch);
                    state = 19;
                }
                else if (ch == '-')
                {
                    word.push_back(ch);
                    state = 23;
                }
                else if (ch == '*')
                {
                    word.push_back(ch);
                    state = 27;
                }
                else if (ch == '\"')
                {
                    state = 51;
                    word.push_back(ch);
                }
                else if (ch == '\'')
                {
                    state = 53;
                    word.push_back(ch);
                }
                else//可直接识别
                {
                    //TODO 很可能有错误
                    if (types_.find(string(&ch, 1)) != types_.end())
                    {
                        tuples_.push_back(node(types_[string(&ch, 1)], string(&ch, 1)));
                        state = 0;
                    }
                    else
                        state = 50;
                }
                break;

            case 1:
                ch = buf.current();
                buf.retrieve(1);
                if (isdigit(ch) || isalpha(ch))
                {
                    word.push_back(ch);
                    state = 1;
                }
                else
                {
                    state = 2;
                }
                break;

            case 2:
                {
                    string s(&*word.begin(), word.size());
                    if (types_.find(s) != types_.end())
                    {
                        tuples_.push_back(node(types_[s], s));
                    }
                    else
                    {
                        tuples_.push_back(node(types_["variable"], s));
                        variables_.push_back(s);
                    }
                    //printf("word :%s\n", s.c_str());
                    //TODO 加入二元组
                    buf.putback();
                    word.clear();
                    state = 0;
                    break;
                }

            case 3:
                ch = buf.current();
                buf.retrieve(1);
                if (isdigit(ch))
                {
                    state = 3;
                    word.push_back(ch);
                }
                else if (ch == '.')
                {
                    state = 5;
                    word.push_back(ch);
                }
                else
                {
                    state = 4;
                }
                break;
            
            case 4:
            {
                string s(&*word.begin(), word.size());
                int num = atoi(s.c_str());
                tuples_.push_back(node(types_["constant"], num));
                ints_.push_back(num);
                //printf("digit :%d\n", num);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }
            
            case 5:
                ch = buf.current();
                buf.retrieve(1);
                if (isdigit(ch))
                {
                    state = 6;
                    word.push_back(ch);
                }
                else
                    state = 50;//TODO ERROR
                break;
            
            case 6:
                ch = buf.current();
                buf.retrieve(1);
                if (isdigit(ch))
                {
                    state = 6;
                    word.push_back(ch);
                }
                else
                    state = 7;
                break;
            
            case 7:
            {
                string s(&*word.begin(), word.size());
                double d = atof(s.c_str());
                tuples_.push_back(node(types_["constant"], d));
                doubles_.push_back(d);
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            //TODO 错误处理
            case 50:
                printf("there are error here\n");//形如3.a是错误的
                buf.putback();
                word.clear();
                exit(-1);
            
            case 8:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 9;
                    word.push_back(ch);
                }
                else if (ch == '<')
                {
                    state = 10;
                    word.push_back(ch);
                }
                else
                {
                    state = 11;
                }
                break;
                
            case 9:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"<="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }
            
            case 10:
            {
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"<<"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }  
            
            case 11:
            {
                //需要放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"<"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }
            
            case 12:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 13;
                    word.push_back(ch);
                }
                else if (ch == '>')
                {
                    state = 14;
                    word.push_back(ch);
                }
                else
                {
                    state = 15;
                }
                break;
            
            case 13:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//">="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 14:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//">>"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 15:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//">"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 16:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 17;
                    word.push_back(ch);
                }
                else
                {
                    state = 18;
                }
                break;
            
            case 17:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"=="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 18:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 19:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 20;
                    word.push_back(ch);
                }
                else if (ch == '+')
                {
                    state = 21;
                    word.push_back(ch);
                }
                else
                {
                    state = 22;
                }
                break;

            case 20:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"+="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 21:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"++"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 22:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"+"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 23:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 25;
                    word.push_back(ch);
                }
                else if (ch == '-')
                {
                    state = 24;
                    word.push_back(ch);
                }
                else
                {
                    state = 26;
                }
                break;

            case 24:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"--"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 25:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"-="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }
            
            case 26:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"-"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 27:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 28;
                    word.push_back(ch);
                }
                else
                {
                    state = 29;
                }
                break;

            case 28:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"*="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 29:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"*"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 30:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 31;
                    word.push_back(ch);
                }
                //TODO 注释
                else if (ch == '/')
                {
                    state = 41;
                }
                
                else if (ch == '*')
                {
                    state = 42;
                }
                else
                {
                    state = 32;
                }
                break;

            case 31:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"/="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 32:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"/"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }
            
            case 33:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '=')
                {
                    state = 34;
                    word.push_back(ch);
                }
                else
                {
                    state = 35;
                }
                break;
            
            case 34:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"!="
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 35:
            {
                //放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_[s], s));//"!"
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                buf.putback();
                word.clear();
                state = 0;
                break;
            }

            //处理注释 //
            case 41:
                word.clear();
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '\n')
                {
                    state = 0;
                    //word.push_back(ch);
                }
                else
                {
                    state = 41;
                }
                break;
            
            //处理注释 /**/
            case 42:
                word.clear();
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '*')
                {
                    state = 43;
                    //word.push_back(ch);
                }
                else
                {
                    state = 42;
                }
                break;
            
            case 43:
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '/')
                {
                    state = 0;
                    //word.push_back(ch);
                }
                else
                {
                    state = 42;
                }
                break;

            case 51:
            {
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '\"')
                {
                    state = 52;
                    word.push_back(ch);
                }
                else
                {
                    state = 51;
                    word.push_back(ch);
                }
                break;
            }

            case 52:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_["constant"], s));
                conststrs_.push_back(s);
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }

            case 53:
            {
                ch = buf.current();
                buf.retrieve(1);
                if (ch == '\'')
                {
                    state = 54;
                    word.push_back(ch);
                }
                else
                {
                    state = 53;
                    word.push_back(ch);
                }
                break;
            }

            case 54:
            {
                //不用放回
                string s(&*word.begin(), word.size());
                tuples_.push_back(node(types_["constant"], s));
                conststrs_.push_back(s);
                //printf("digit :%f\n", d);
                //TODO 加入二元组
                //buf.putback();
                word.clear();
                state = 0;
                break;
            }
        }  
    }
}

int main(void)
{
    wordParser w;
    w.pretreat();
    //w.upload();
    w.parse();
    w.printClass();
    w.printInt();
    w.printDouble();
    w.printStr();
    w.printVariable();
    printf("\n解析后得到的二元组集合:\n");
    w.print();
    return 0;
}