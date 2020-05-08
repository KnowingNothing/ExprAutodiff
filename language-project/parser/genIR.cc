/****************************************************/
/* File: genIR.cc                                   */
/* to generate IR code                              */
/****************************************************/

#include "genIR.h"
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
using namespace std;
ofstream ofile;
set<string> varset;
vector<string> varlist;
vector<string> exprlist;



string tokentostring(TokenType token)
{
    switch (token)
    {
    case PLUS:
        return string("BinaryOpType::Add,");
    case TIMES:
        return string("BinaryOpType::Mul,");
    case MINUS:
        return string("BinaryOpType::Sub,");
    case DIV:
        return string("BinaryOpType::Div,");
    case MOD:
        return string("BinaryOpType::Mod,");
    case FLOORDIV:
        return string("BinaryOpType::FloorDiv,");
    default:
        cout<<"error token"<<endl;
        return string();
    }
}


void genDomIndex(TreeNode* clist,TreeNode* alist, int isreduce)
{
    while(clist != NULL && alist != NULL)
    {
        if(alist->nodekind == IdK)
        {
            if(varset.find(alist->name)==varset.end())
            {
                varlist.push_back(alist->name);
                varset.insert(alist->name);
                ofile<<"Expr dom_"<<alist->name<<" = Dom::make(index_type, 0, "<<clist->val<<");"<<endl;
                ofile<<"Expr "<<alist->name<<" = Index::make(index_type, \""<<alist->name<<"\", dom_"<<alist->name;
                if(isreduce)
                    ofile<<", IndexType::Reduce);"<<endl;
                else
                    ofile<<", IndexType::Spatial);"<<endl;
            }
        }
        clist = clist->sibling;
        alist = alist->sibling;
    }
    if(clist!=NULL||alist!=NULL)
    {
        cout<<"different length of clist and alist"<<endl;
    }
}

string genIdexpr(TreeNode* alist)
{
    if(alist->nodekind == IdK)
    {
        return alist->name;
    }
    else if (alist->nodekind == IdexprK)
    {
        string left = genIdexpr(alist->child[0]);
        string right = genIdexpr(alist->child[1]);
        string ret = "Binary::make(index_type, "+tokentostring(alist->op)+left+","+right+")";
        return ret;
    }
    else
    {
        string ret = "IntImm::make(index_type, "+to_string(alist->val)+")";
        return ret;
    }
}

void domassist(TreeNode *tree, int inright)
{
    if (tree->nodekind == RhsK)
    {
        domassist(tree->child[0],1);
        domassist(tree->child[1],1);
    }
    else if(tree->nodekind == TrefK)
    {
        char* thename = tree->child[0]->child[0]->name;
        TreeNode* clist = tree->child[0]->child[1];
        TreeNode* alist = tree->child[1];
        genDomIndex(clist,alist,inright);
    }
}

string genIR(TreeNode *tree)
{
    while (tree != NULL)
    {
        if (tree->nodekind == StmtK)
        {
            domassist(tree->child[0],0);
            domassist(tree->child[1],1);
            string left = genIR(tree->child[0]);
            string right = genIR(tree->child[1]);
            ofile <<"Stmt main_stmt = Move::make("<<endl;
            ofile <<left<<","<<endl;
            ofile <<"Binary::make(data_type, BinaryOpType::Add, ";
            ofile <<left<<",";
            ofile <<right<<"),"<<endl;
            ofile <<"MoveType::MemToMem\n);"<<endl;
        }
        else if (tree->nodekind == RhsK)
        {
            string left = genIR(tree->child[0]);
            string right = genIR(tree->child[1]);
            string ret = "Binary::make(data_type, ";
            ret = ret + tokentostring(tree->op)+left+", "+right+")";
            return ret;
        }
        else if(tree->nodekind == TrefK)
        {
            char* thename = tree->child[0]->child[0]->name;
            TreeNode* clist = tree->child[0]->child[1];
            TreeNode* alist = tree->child[1];
            ofile<<"Expr expr_"<<thename<<"= Var::make(data_type, \""<<thename<<"\",\n{";
            int isfirst = 1;
            while (alist)
            {
                if(!isfirst)
                        ofile<<", ";
                ofile<<genIdexpr(alist);
                isfirst = 0;
                alist = alist->sibling;
            }
            ofile<<"},\n{";
            isfirst = 1;
            while (clist)
            {
                if(!isfirst)
                        ofile<<", ";
                ofile<<clist->val;
                isfirst = 0;
                clist = clist->sibling;
            }
            
            ofile<<"});"<<endl;
            string ret = thename;
            ret = "expr_"+ret;
            exprlist.push_back(ret);
            return ret;
        }
        if(tree->nodekind == StmtK)
        {
            ofile<<"Stmt loop_nest = LoopNest::make({";
            int isfirst = 1;
            for(int i=0;i<varlist.size();i++)
            {
                if(!isfirst)
                    ofile<<", ";
                isfirst = 0;
                ofile<< varlist[i];
            }
            ofile<<"}, {main_stmt});"<<endl;

            ofile<<"Group kernel = Kernel::make(\"";
            if(outname)
                ofile<<outname;
            else
                ofile<<"out";
            ofile<<"\", {";
            isfirst = 1;
            for(int i=1;i<exprlist.size();i++)
            {
                if(!isfirst)
                    ofile<<", ";
                isfirst = 0;
                ofile<< exprlist[i];
            }
            ofile<<"}, {"<<exprlist[0]<<"}, {loop_nest}, KernelType::CPU);"<<endl;

            ofile<<"IRVisitor visitor;"<<endl;
            ofile<<"kernel.visit_group(&visitor);"<<endl;
            ofile<<"IRMutator mutator;"<<endl;
            ofile<<"kernel = mutator.mutate(kernel);"<<endl;
            ofile<<"IRPrinter printer;"<<endl;
            ofile<<"std::string code = printer.print(kernel);"<<endl;
            ofile<<"std::cout << code;"<<endl;

            varset.clear();
            varlist.clear();
            exprlist.clear();
            tree = tree->sibling;
            ofile<<endl<<endl;
        }
        else
            break;
    }

    return string();
}

void outinit(TreeNode *tree)
{
    if(outname)
        ofile.open(outname);
    else
        ofile.open("out.cc");
    ofile<<"Type index_type = Type::int_scalar(32);"<<endl;
    ofile<<"Type data_type = Type::float_scalar(32);"<<endl;
    genIR(tree);
}