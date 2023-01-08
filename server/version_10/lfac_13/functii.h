#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int total_variabile = 0;
int total_functii = 0;
int parametri_expresie = 0;
char* memoID;
char tipData[50];
char parametri[50];
struct varNumeStruct
{
    int valoareInt;
    float valoareFloat;
    char valoareChar;
    char *valoareString;

    int isConst; // 0 - NU | 1 - DA
    char *nume;
    char *tip_date;
    char *scope;
    char *valoare;
    int eInitializata;
} variabile[20];
struct expresie
{
    int valoareInt;
    float valoareFloat;
    char valoareChar;
    char *valoareString;

    char *tip_date;
};

struct functie
{
    char *nume;
    char *tip_date;
    struct parametru{
        char* param_type;
        char* param_id;
    }parametri[20][20];
    int nr_parametri;
} functii[20];



struct Clase
{
    char *nume;
} clase[20];

extern int yylineno;

void adaugaID(char *id, char *id_data_type)
{
    variabile[total_variabile].nume = id;
    variabile[total_variabile].tip_date = id_data_type;
    total_variabile++;
}


void adaugaConstID_int(char *id, int val)
{
    variabile[total_variabile].nume = id;
    variabile[total_variabile].valoareInt = val;
    variabile[total_variabile].isConst = 1;
    total_variabile++;
}

int getConst(char *id)
{
    for (int i = 0; i < total_variabile; i++)
    {
        if (strcmp(id, variabile[i].nume) == 0)
        {
            if (variabile[i].isConst == 1)
                return 1;
            else
                return 0;
        }
    }
}

int verificareLocalizare(char *id, char *scope)
{
    for (int i = 0; i < total_variabile; i++)
    {
        if (strcmp(id, variabile[i].nume) == 0 && strcmp(scope, "main") == 0 && strcmp("global", variabile[i].scope) == 0) // variabila e declarata global deja
        {
            return 2;
        }
        if (strcmp(id, variabile[i].nume) == 0 && strcmp(scope, variabile[i].scope) == 0) // duplicat
        {
            return 1;
        }
    }
    return 0; // variabila nu este declarata
}
void redeclarareScope(char *tip, char *id, char *scope)
{
    for (int i = 0; i < total_variabile; i++)
    {

        if (strcmp(id, variabile[i].nume) == 0)
        {
            if (variabile[i].isConst == 0)
            {
                if (strcmp(variabile[i].tip_date, tip) == 0)
                {
                    variabile[i].scope = "main";
                    if (strcmp(variabile[i].tip_date, "int") == 0)
                        variabile[i].valoareInt = 0;
                    else if (strcmp(variabile[i].tip_date, "float") == 0)
                        variabile[i].valoareFloat = 0;
                    else if (strcmp(variabile[i].tip_date, "string") == 0)
                        variabile[i].valoareString = " ";
                    else if (strcmp(variabile[i].tip_date, "char") == 0)
                        variabile[i].valoareChar = ' ';
                    break;
                }
                else
                {
                    printf("O variabila declarata global nu isi poate schimba tipul, linia %d\n", yylineno);
                    exit(1);
                }
            }
            else
            {
                printf("O variabila declarata const nu poate fi redeclarata, linia %d\n", yylineno);
                exit(1);
            }
        }
    }
}
void redeclarareScopeVal(char *tip, char *tipValoare, char *id, char *valoare, char *scope)
{
    for (int i = 0; i < total_variabile; i++)
    {
        if (strcmp(id, variabile[i].nume) == 0)
        {
            if (variabile[i].isConst == 0)
            {
                if (strcmp(variabile[i].tip_date, tip) == 0)
                {
                    if (strcmp(variabile[i].tip_date, tipValoare) == 0)
                    {
                        variabile[i].scope = "main";
                        if (strcmp(variabile[i].tip_date, "int") == 0)
                            variabile[i].valoareInt = atoi(valoare);
                        else if (strcmp(variabile[i].tip_date, "float") == 0)
                            variabile[i].valoareFloat = atof(valoare);
                        else if (strcmp(variabile[i].tip_date, "string") == 0)
                            variabile[i].valoareString = valoare;
                        else if (strcmp(variabile[i].tip_date, "char") == 0)
                            variabile[i].valoareChar = atoi(valoare);
                        break;
                    }
                    else
                    {
                        printf("Nu pot exista asignari intre tipuri de date diferite, linia %d\n", yylineno);
                        exit(1);
                    }
                }
                else
                {
                    printf("O variabila declarata global nu isi poate schimba tipul, linia %d\n", yylineno);
                    exit(1);
                }
            }
            else
            {
                printf("O variabila declarata const nu poate fi redeclarata, linia %d\n", yylineno);
                exit(1);
            }
        }
    }
}

int eDuplicat(char *id)
{
    for (int i = 0; i < total_variabile; i++)
    {
        if (strcmp(id, variabile[i].nume) == 0)
        {
            return 1;
        }
    }
    return 0;
}
char *verificareTip(char *id, char *scope)
{
    for (int i = 0; i < total_variabile; i++)
    {
        if (strcmp(id, variabile[i].nume) == 0 && strcmp(scope, variabile[i].scope) == 0)
        {
            return variabile[i].tip_date;
        }
    }
}
int verificareTipExpresie(char *tipOp1, char *tipOp2)
{
    printf("s -%s s-%s\n", tipOp1, tipOp2);
    if (strcmp(tipOp1, tipOp2) == 0)
        return 1;
    else
        return 2;
}
void declarareId(char *tip, char *id, char *scope, int isConst)
{

    variabile[total_variabile].nume = id;
    variabile[total_variabile].isConst = isConst;
    variabile[total_variabile].scope = scope;
    variabile[total_variabile].valoareString = " ";
    variabile[total_variabile].valoareChar = ' ';
    variabile[total_variabile].tip_date = tip;
    total_variabile++;
}
int checkAssign(char *tipData, char *tipValoare)
{
    if (strcmp(tipData, tipValoare) == 0)
        return 1;
    else
        return 0;
}
void declarareIdVal(char *tip, char *id, char *valoare, char *scope, int isConst)
{
    variabile[total_variabile].nume = id;
    variabile[total_variabile].isConst = isConst;
    variabile[total_variabile].scope = scope;
    variabile[total_variabile].tip_date = tip;

    if (strcmp(variabile[total_variabile].tip_date, "int") == 0)
        variabile[total_variabile].valoareInt = atoi(valoare);
    else if (strcmp(variabile[total_variabile].tip_date, "float") == 0)
        variabile[total_variabile].valoareFloat = atof(valoare);
    else if (strcmp(variabile[total_variabile].tip_date, "string") == 0)
    {
        variabile[total_variabile].valoareString = valoare;
    }
    else if (strcmp(variabile[total_variabile].tip_date, "char") == 0)
        variabile[total_variabile].valoareChar = atoi(valoare);

    total_variabile++;
}
void asignare(char *id, char *valoare, char *scope)
{
    for (int i = 0; i < total_variabile; i++)
    {
        if (strcmp(id, variabile[i].nume) == 0 && strcmp(scope, variabile[i].scope) == 0)
        {

            if (variabile[i].isConst == 0)
            {
                if (strcmp(variabile[i].tip_date, "int") == 0)
                    variabile[i].valoareInt = atoi(valoare);
                else if (strcmp(variabile[i].tip_date, "float") == 0)
                    variabile[i].valoareFloat = atof(valoare);
                else if (strcmp(variabile[i].tip_date, "string") == 0)
                    variabile[i].valoareString = valoare;
                else if (strcmp(variabile[i].tip_date, "char") == 0)
                    variabile[i].valoareChar = atoi(valoare);
                break;
            }
            else
            {
                printf("O variabila declarata const nu mai poate primi alta valoare, linia %d\n", yylineno);
            }
        }
    }
}
void afisare()
{
    FILE *fp;
    
    fp = fopen("symbol_table.txt", "w");
    fprintf(fp, "Id - Tip - Scope - Valoare - Constant\n");
    for (int i = 0; i < total_variabile; i++)
    {
        fprintf(fp, "%s - %s - %s - ", variabile[i].nume, variabile[i].tip_date, variabile[i].scope);
        if (strstr(verificareTip(variabile[i].nume, variabile[i].scope), "int"))
        {
            fprintf(fp, "%d - ", variabile[i].valoareInt);
        }
        if (strstr(verificareTip(variabile[i].nume, variabile[i].scope), "float"))
        {
            fprintf(fp, "%f - ", variabile[i].valoareFloat);
        }
        if (strstr(verificareTip(variabile[i].nume, variabile[i].scope), "string"))
        {
            fprintf(fp, "%s - ", variabile[i].valoareString);
        }
        if (strstr(verificareTip(variabile[i].nume, variabile[i].scope), "char"))
        {
            fprintf(fp, "%c - ", variabile[i].valoareChar);
        }
        fprintf(fp, "%d\n", variabile[i].isConst);
    }
    printf("Tabelul de simboluri a fost creat.\n");
    fclose(fp);
}
void printeaza(char *val)
{
    printf("val = %s\n", val);
}

char *itoa(int number)
{
    char *buffer = malloc(30);
    snprintf(buffer, sizeof(buffer), "%d", number);
    return buffer;
}
char *ftoa(float number)
{
    char *buffer = malloc(30);
    snprintf(buffer, sizeof(buffer), "%f", number);
    return buffer;
}


//             FUNCTII              //


void adauga_parametru(char* id, char* paramType, char* paramID)
{
    printf("total:%d\n", total_functii);
    for(int i = 0; i < total_functii; i++)
    {
        if(functii[i].nume == id)
        {
            printf("nume: %s\n", functii[i].nume);
            functii[i].parametri[0]->param_type = paramType;
            printf("param type: %s\n",functii[i].parametri[0]->param_type);
            functii[i].parametri[0]->param_id = paramID;
            printf("param id: %s\n",functii[i].parametri[0]->param_id);
            functii[i].nr_parametri++;
        }
    }
}
void declara_functie(char* data_type, char* id)
{
    functii[total_functii].nume = id;
    functii[total_functii].tip_date = data_type;
    functii[total_functii].nr_parametri = 0;
    total_functii++;
}

int verificaDuplicat_functie(char* id)
{
    for(int i = 0; i < total_functii; i++)
    {
        if(strcmp(id, functii[i].nume) == 0)
            return 1;
    }
    return 0;
}

void afisare_functii()
{
    FILE *fp;
    fp = fopen("functions_table.txt", "w");
    fprintf(fp,"type  -  id  -  params\n\n");
    for(int i = 0; i < total_functii; i++)
    {
      fprintf(fp, "%s | ", functii[i].tip_date);
      fprintf(fp, "%s | ", functii[i].nume);
      for(int j = 0; j < 1; j++)
      {
        fprintf(fp, "%s  ", functii[i].parametri[j]->param_type);
        fprintf(fp, "%s  |\n", functii[i].parametri[j]->param_id);  
      }
    }
    printf("Tabelul de functii a fost creat.\n");
    fclose(fp);
}