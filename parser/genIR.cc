/****************************************************/
/* File: genIR.cc                                   */
/* to generate IR code                              */
/****************************************************/

#include <set>
#include <vector>
#include <map>

#include "genIR.h"
#include "globals.h"
#include "util.h"


// #include "IR.h"
// #include "IRMutator.h"
// #include "IRVisitor.h"
// #include "IRPrinter.h"
// #include "type.h"


using namespace std;
using namespace Boost::Internal;


extern  "C"
{
    extern TreeNode *parse(char* name);
};

// ofstream ofile;
set<string> varset;
vector<string> varlist;
vector<string> exprlist;
map<string,Expr> indexset;

Type index_type = Type::int_scalar(32);
Type data_type = Type::float_scalar(32);

BinaryOpType tokentoop(TokenType token)
{
    switch (token)
    {
    case PLUS:
        return BinaryOpType::Add;
    case TIMES:
        return BinaryOpType::Mul;
    case MINUS:
        return BinaryOpType::Sub;
    case DIV:
        return BinaryOpType::Div;
    case MOD:
        return BinaryOpType::Mod;
    case FLOORDIV:
        return BinaryOpType::FloorDiv;
    default:
        cout<<"error token"<<endl;
        return BinaryOpType::Add;
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

                Expr dom = Dom::make(index_type, 0, clist->val);
                Expr index;
                if(isreduce)
                    index = Index::make(index_type, alist->name, dom, IndexType::Reduce);
                else
                    index = Index::make(index_type, alist->name, dom, IndexType::Spatial);
                indexset.insert(pair<string, Expr>(string(alist->name),index));
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

Expr genIdexpr(TreeNode* alist)
{
    if(alist->nodekind == IdK)
    {
        map<string,Expr>::iterator iter1;
        iter1 = indexset.find(string(alist->name));
        if(iter1 == indexset.end())
        {
            cout<<"error: no such index"<<endl;
        }
        return iter1->second;
    }
    else if (alist->nodekind == IdexprK)
    {
        Expr left = genIdexpr(alist->child[0]);
        Expr right = genIdexpr(alist->child[1]);
        Expr ret = Binary::make(index_type, tokentoop(alist->op),left,right);
        return ret;
    }
    else
    {
        return IntImm::make(index_type,alist->val);
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
        TreeNode* clist = tree->child[0]->child[1];
        TreeNode* alist = tree->child[1];
        genDomIndex(clist,alist,inright);
    }
}


Expr genIR(TreeNode *tree)
{
    if (tree->nodekind == RhsK)
    {
        Expr left = genIR(tree->child[0]);
        Expr right = genIR(tree->child[1]);
        Expr ret = Binary::make(data_type, tokentoop(tree->op),left,right);
        return ret;
    }
    else if(tree->nodekind == TrefK)
    {
        char* thename = tree->child[0]->child[0]->name;
        TreeNode* clist = tree->child[0]->child[1];
        TreeNode* alist = tree->child[1];

        std::vector<Expr> _args;
        std::vector<uint64_t> _shape;

        while (alist)
        {
            _args.push_back(genIdexpr(alist));
            alist = alist->sibling;
        }
        while (clist)
        {
            _shape.push_back(clist->val);
            clist = clist->sibling;
        }

        return Var::make(data_type,thename,_args,_shape);
    }
    else
    {
        cout<<"error in genIR"<<endl;
    }
}

Stmt getLoopnest(TreeNode *tree)
{
    domassist(tree->child[0],0);
    domassist(tree->child[1],1);
    Expr left = genIR(tree->child[0]);
    Expr right = genIR(tree->child[1]);
    Stmt main_stmt = Move::make(left,
                                Binary::make(data_type, BinaryOpType::Add, left, right),
                                MoveType::MemToMem);
    std::vector<Expr> _index_list;
    for(int i=0;i<varlist.size();i++)
    {
        map<string,Expr>::iterator iter1;
        iter1 = indexset.find(varlist[i]);
        _index_list.push_back(iter1->second);
    }

    Stmt loop_nest = LoopNest::make(_index_list,{main_stmt});
    return loop_nest;
}

Stmt outinit(char* name)
{
    TreeNode* tree = parse(name);
    return getLoopnest(tree);
}

