#include <iostream>
#include <stack>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <algorithm>

using namespace std;


/*
AUTHORS:
ALI KAAN BIBER              
YAVUZ SAMET TOPCUOGLU
*/


int counter = 1; //counts the number of temporary variables used in the program.
int mychoose_counter = 0; //counts the number of choose calls in order to differentiate choose calls.
int chooseCount = 0; //counts the number of chooses during syntax check. 

map<string,int> vars; //maps variable names to their values.

ofstream outFile; //file to be outputted.

bool commacheck(string str); //checks whether the number of chooses are consistent with the number of commas.
int precedence(char c); //gives predence to operations to form postfix notation.
bool isTemp(string s); //checks whether a string is temporary.
string inToPost(string s); //transforms infix notation to postfix notation.
bool syntaxPostFix(string str); //checks the syntax of postfix notation expression.
string spaceCanceller(string text); //cancels spaces between tokens.
queue<string> evaluate(string postfix); //stores variables in the postfix notation.
bool isVariable(string s); //checks whether a string is variable.
void expressionToIR(queue<string>q , stack<string> st , map<string,int>vars, bool fromAssignment, string variable_name); //outputs the given expression in wanted llvm format to the output file.
int syntaxCheck(ifstream &inFile2); //checks the syntax of the given input file.
string chooseSyntaxChecker(string basicString); //checks the syntax of choose  calls.
int choose(string text); //progresses choose calls.

int main(int argc, char const *argv[]) {

    ifstream inFile,inFile2; 
    inFile.open(argv[1]); //input file for translation.
    inFile2.open(argv[1]); //input file for syntax check.

    string fileName = argv[1];		//output file should match the given input file
    fileName= fileName.substr(0,fileName.find(".")) + ".ll";
    outFile.open(fileName); //output file.

    //if the file has a syntax error, terminate the program.
	int res = syntaxCheck(inFile2); 
    if(res != -1){
        outFile << "; ModuleID = 'mylang2ir'\n"
               "declare i32 @printf(i8*, ...)\n"
               "@print.str = constant [23x i8] c\"Line %d: syntax error\\0A\\00\"\n"
               "\n"
               "define i32 @main() {\n"
               "\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([23 x i8]* @print.str, i32 0, i32 0), i32 "<<res<< "  )\n"
               "\tret i32 0\n}"<<endl;
        return 0;
    }

    //beginning of the llvm program.
    outFile << "; ModuleID = 'mylang2ir'\n"
               "declare i32 @printf(i8*, ...)\n"
               "@print.str = constant [4 x i8] c\"%d\\0A\\00\"\n"
               "\n"
               "define i32 @main() {"<< endl;

    //allocation and storage of variables.
    for(pair<string,int> p:vars){
        outFile << "\t%"<<p.first<<" = alloca i32"<<endl;
        outFile << "\tstore i32 0, i32* %"<<p.first<<endl;
    }

    //allocation and storage of choose differentiators.
    for(int i=0;i<chooseCount;i++){
    	outFile << "\t\%datdiri3datdat1daaatdat" << i <<" = alloca i32"<< endl;
        outFile << "\tstore i32 0, i32* \%datdiri3datdat1daaatdat"<< i << endl;
    }

    bool in_if = false;
    bool in_while = false;

    int ifNo = 0;
    int whileNo = 0;


    string line; 

    while(getline(inFile, line)){ //line by line reading from the input.

        string infix;

        string line_sc = spaceCanceller(line); //delete the spaces.

        //if there is a comment sign, then ignore the rest of the line.
        int index_of_comment = line_sc.find("#");
        if(index_of_comment != -1){
            line_sc = line_sc.substr(0, index_of_comment);
        }

        //while line contains choose method,
        int index_of_choose = line_sc.find("choose(");
        while(index_of_choose != -1){
        	//finds the inside of the choose parenthesis.
            string examine = line_sc.substr(index_of_choose+7);
            int balance_check = 1;
            int i;
            for(i=0; i<examine.length(); i++){
                if(examine.at(i) == '(') balance_check++;
                else if(examine.at(i) == ')') balance_check--;
                if(balance_check == 0) break;
            }
            string inside_of_parenthesis = examine.substr(0,i); //inside of choose.
            
            //replace the choose part with the temporary value contains it.
            int replacement = choose(inside_of_parenthesis);
            string replacementt = "datdiri3datdat1daaatdat" + to_string(replacement);
            line_sc.replace(line_sc.begin()+index_of_choose, line_sc.begin()+index_of_choose+inside_of_parenthesis.length()+8, replacementt.begin(), replacementt.end());
        	index_of_choose =  line_sc.find("choose(");
        }


        //if line contains assignment,
        int index_of_equals = line_sc.find("=");
		if(index_of_equals != -1){

			//finds variable.
            string variable_name;
            variable_name = line_sc.substr(0,index_of_equals);

            //if a variable does not exist, create it.
            if(vars.count(variable_name) == 0){
                vars.insert(pair<string,int>(variable_name,0));
                outFile << "\t%"<<variable_name<<" = alloca i32"<<endl;
                outFile << "\tstore i32 0, i32* %"<<variable_name<<endl;
			}

			//find the infix expression and convert into postfix.
            infix = line_sc.substr(index_of_equals+1);
			string postfix = inToPost(infix);

            //translates to llvm code after finding postfix.
            queue<string> q;
            q=evaluate(postfix);
            stack<string> st;
            expressionToIR(q , st , vars,true, variable_name);
        }


        //if line contains if,
        string if_checker = line_sc.substr(0,3);
        if(if_checker == "if("){
			
			in_if = true;
            outFile << "\tbr label %ifcond" << ifNo << "\n\nifcond"<< ifNo <<":" << endl; 
            
            //finds the expression inside the if statement.
            int index_of_parenthesis = line_sc.find(")");
            string expression = "";
            int count=0;
            for(int i=3;i<line_sc.length()-2;i++){
                if(line_sc.at(i)=='('){
                    count++;
                }else if(line_sc.at(i)==')'){
                    count--;
                    if(count==-1){
                        break;
                    }
                }
                expression+=line_sc.at(i);
            }
            string postfix = inToPost(expression); //transforms into postfix.
           	
           	//translates to llvm code after finding postfix.
            queue<string> q;
            q=evaluate(postfix);
            stack<string> st;
            expressionToIR(q , st , vars,false, "");

            //make comparison.
            outFile << "\t%gokberki" <<counter <<" = icmp ne i32 %gokberki"<<counter-1<<" , 0" <<endl; 
            counter++;
            outFile << "\tbr i1 %gokberki"<< counter-1<<", label \%ifbody"<< ifNo <<", label \%ifend"<<ifNo <<endl;
            outFile << "\nifbody"<<ifNo<<":" << endl;

        }


        //if line contains while,
        string while_checker = line_sc.substr(0,6);
        if(while_checker == "while("){

            in_while = true;
            outFile << "\tbr label %whcond"<<whileNo<<"\n\nwhcond"<<whileNo<<":" << endl;
            
            //finds the expression inside the if statement.
            int index_of_parenthesis = line_sc.find(")");
            string expression = "";
            int count=0;
            for(int i=6;i<line_sc.length()-2;i++){
                if(line_sc.at(i)=='('){
                    count++;
                }else if(line_sc.at(i)==')'){
                    count--;
                    if(count==-1){
                        break;
                    }
                }
                expression+=line_sc.at(i);
            }
            string postfix = inToPost(expression); //transforms into postfix.
            
            //translates to llvm code after finding postfix.
            queue<string> q;
            q=evaluate(postfix);
            stack<string> st;
            expressionToIR(q , st , vars,false, "");

            //make comparison.
			outFile << "\t%gokberki" <<counter <<" = icmp ne i32 %gokberki"<<counter-1<<" , 0" <<endl;
            counter++;
            outFile << "\tbr i1 %gokberki"<< counter-1<<", label %whbody"<<whileNo<<", label %whend"<<whileNo <<endl;
            outFile << "\nwhbody"<<whileNo<<":" << endl;


        }


        //if line contains print,
        string print_checker = line_sc.substr(0,6);
        if(print_checker == "print("){

            int index_of_parenthesis = line_sc.find(")");
            string expression = "";
            int count=0;
            for(int i=6;i<line_sc.length();i++){
                if(line_sc.at(i)=='('){
                    count++;
                }else if(line_sc.at(i)==')'){
                    count--;
                    if(count==-1){
                        break;
                    }
                }
                expression+=line_sc.at(i);
            }
       		string postfix = inToPost(expression); //transforms into postfix.
            
       		//translates to llvm code after finding postfix.
            queue<string> q;
            q=evaluate(postfix);
            stack<string> st;
            expressionToIR(q , st , vars,false, "");
            
            //calls print function.
            outFile << "\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4x i8]* @print.str, i32 0, i32 0), i32 %gokberki"<<counter-1<< "  )" << endl; 
        }

        //if line contains curly bracket.
        string curly_bracket_checker = line_sc.substr(0,1);
        if(curly_bracket_checker == "}"){

        	//if this is the end of the if,
            if(in_if){
                in_if = false;
                outFile << "\tbr label \%ifend"<<ifNo << endl;
                outFile << "\nifend"<<ifNo<<":" << endl;
                ifNo++;
            }
            //if this is the end of the while,
            else if(in_while){
                in_while = false;
                outFile << "\tbr label %whcond"<<whileNo << endl;
                outFile << "\nwhend"<<whileNo<<":" << endl;
                whileNo++;
            }
        }

    }

    //finish th program.
    outFile << "\tret i32 0\n}" <<endl;

    return 0;
}

int precedence(char c) {	//returns an integer depending on operation precedence to create postfix notations

    if(c == '*' || c == '/'){	//* and / have
        return 2;
    }
    else if(c == '+' || c == '-'){
        return 1;
    }
    else {
        return -1;
    }

}

string inToPost(string s){	//transforms the given infix string into a postfix string
    
    stack<char> st;			//character stack to store operators 
    int length = s.length();		
    string postfix = "";	//string to return when the process ends
    s = spaceCanceller(s);	//get rid of the unnecessary spaces first

    for(int i=0;i<length;i++) {	//for loop to traverse each character in the string

        char c = s[i];			//current char
        
        //if current char is valid create a new string to store the variable name or value
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >='0' && c<='9')) {
           
            string variableName="";
            variableName+=c;

            postfix += c;			//append current character to the end of postfix string

            int nextChars = i+1;	//if variable name or the number has more than 1 character find the rest
            if(nextChars!=length) {
            	//checks the next character if valid concatanate with the previous character
                while (((s[nextChars] >= 'a' && s[nextChars] <= 'z') || (s[nextChars] >= 'A' && s[nextChars] <= 'Z') ||
                        (s[nextChars] >= '0' && s[nextChars] <= '9')) && nextChars < length) {

                    variableName+=s[nextChars];			
                    postfix += s[nextChars];			//update the postfix string aswell
                    i++;								//increment the variable used in for loop since we already checked the next one
                    nextChars++;						//move to the next character to check
                }

        }

        if(variableName == "while" || variableName == "choose" || variableName == "if" || variableName == "print") return "";


        //integer followed by a character is a syntax error
        if(variableName.at(0)<='9' && variableName.at(0)>='0'){ //if starts with an integer check the other characters
            for(int i=1;i<variableName.length();i++){

                if(!(variableName.at(i)<='9' && variableName.at(i)>='0')) {     //if one of the next characters is not an integer then error
                    return "";
                }
            }
        }


        if(isVariable(variableName)&&vars.count(variableName)==0){  
                    //if variable is not in the variable map insert it so that in main we can allocate space for them
                    vars.insert(pair<string,int>(variableName,0));
                }

        }	else if( c == '('){		//if the current character is opening paranthesis

            if(i!=length-1){		//check whether it is the end of string

                if(s.at(i+1)==')'){	//if closing paranthesis comes just after opening paranthesis then syntax error

                    return "";		//to distinguish syntax error returns null string
                }
            }
            st.push('(');			//no problem then push it to character stack

        }	else if( c== ')'){		//if the current character is closing paranthesis

            while( !st.empty() && st.top() != '(' ){	

                char t = st.top();	//pop from stack until find the matched opening paranthesis and update postfix
                st.pop();
                postfix+=" ";
                postfix+=t;
            }

            if(st.top() =='('){		//if opening paranthesis is on top of stack pop it
                char t=st.top();
                st.pop();
            }

        }   else if(c == ','){		//if there is comma in the string also error

            return "";
        }
        	else{			//means current character is an operator

            while ( !st.empty() && precedence(c) <= precedence(st.top())){	
            //if operator has lower precedence than the one in the stack pop from stack and append it to postfix string

                char t= st.top();
                st.pop();
                postfix+=" ";
                postfix +=t;
            }

            st.push(c);		//push the current operator to stack
        }
        postfix+= " ";		//add spaces after each iteration so that tokenization can be done later
    }

    while( !st.empty() ){	//if there is any operator in stack pop and append it to postfix string
        char t=st.top();
        st.pop();
        postfix+=" ";
        postfix+=t;

    }

    return postfix;		//return the final string in postfix notation
}

string spaceCanceller(string text){		//gets rid of unwanted spaces and "tab" characters

    string res = "";					//string to be returned 

    for(int i=0; i<text.length(); i++){	//traverse each character to detect spaces or tabs

        if(text[i]=='\t'){				//move on if it is a tab
            continue;
        }
        else if(text[i] != ' '){		//append the character to string if current character is not space
            res += text[i];
        }
    }
    return res;							//return the final string
}

bool syntaxPostFix(string str) {		//checks whether the postfix notation is valid or not

	//variables to store operator and operands count
    int operatorCount = 0;				
    int tokenCount = 0;

    for(int i=0;i<str.length();i++){	//iterate through the postfix string 
        
        char c = str.at(i);

        if(c == ' '){					//continue if it is a space
            continue;
        }
        else if( c == '(' || c == ')'){	
        //there should be no paranthesis in postfix notation 
        //if given expression has unmatched paranthesis then postfix contains paranthesis which is an error
            return false;
        }
        else if( c == '+' || c == '*' || c == '/' || c =='-'){	//if current character is an operator increment operatorCount

            operatorCount++;
        }
        else if(c=='='){	//in a postfix there cant be any '=' character

            return false;
        }
        else {	//if we enter else part it is a token
            if(i!=str.length()-1) {
                if (str.at(i + 1) == ' ') {	//if it is followed by a space then it is a token
                    tokenCount++;
                }
            }
        }

    }

    if(operatorCount + 1 != tokenCount){	// token count should be one more than operator count if not error
        return false;
    }
    return true;			//otherwise valid postfix

}

queue<string> evaluate(string postfix) {	//returns a queue to be used in translating mylang to LLVM code
    
    stringstream ss;	//stringstream to read tokenized postfix

    queue <string> q;	//queue to be returned

    string temp;		//temporary string to store tokens in postfix

    ss << postfix;		//ss will find the tokens in postfix

    while(ss >> temp){	//find the next token and push it to the queue

        q.push(temp);
    }

    return q;		//return queue
}

bool isVariable(string s) {		//returns if given string is an integer or a variable

    if(s.at(0) >= '0' && s.at(0)<='9'){		//if first character is an integer then it is an integer

        return false;
    }
    else {									//else variable
    	return true;
    }

}

bool isTemp(string s) {		//returns if a variable is temporary in LLVM 
	//in other words if given string is in form "gokberki" followed by an integer then true else false

    if(s=="gokberki"){		//an edge case needs be considered
        return false;
    }

    if(s.substr(0,8) == "gokberki"){		//It should start with 'gpkberki'

        for(int i=8;i<s.length();i++){ 	//rest of the string should be numbers

            if( !(s.at(i)>='0' && s.at(i)<='9') ){	//if not followed by a number then not temporary

                return false;
            }
        }
    }
    else {	
    	//if not start with 'gokberki' not temporary
        return false;
    }

    return true;	//if we reach here then temporary
}

void expressionToIR(queue<string> q, stack<string> st , map<string, int> vars, bool fromAssignment,string variable_name) { //translates the expression to ir code.

    while(!q.empty()){ //if not reached the end of the postfix. 

    	//first element in the postfix notation.
        string temp=q.front(); 
        q.pop();

        if(temp != "+" && temp != "/" && temp != "*" && temp != "-" ){ //if the element is not operator,

            st.push(temp); //add the element to the operand stack.
        }
        else { //if the element is an operator,

        	//operands to be operated.
            string s1,s2;
            s1=st.top();
            st.pop();
            s2=st.top();
            st.pop();

            //check if the operands are variables.
            bool var1 = isVariable(s1);
            bool var2 = isVariable(s2);

            //if the variables are not declared, make allocation and storage.
            if(var1 && vars.count(s1)==0 && !isTemp(s1)){
                outFile << "\t%"<<s1<<" = alloca i32"<<endl;
                outFile << "\tstore i32 0, i32* %"<<s1<<endl;
            }
            if(var2 && vars.count(s2)==0 && !isTemp(s2)){
                outFile << "\t%"<<s2<<" = alloca i32"<<endl;
                outFile << "\tstore i32 0, i32* %"<<s2<<endl;
            }

            //if the operands are variables,
            if(var1 && var2){

            	//if they are not temp already, then load them to a new temp.
                if(!isTemp(s1)){
                    outFile << "\t%gokberki"<<counter <<" = load i32* %"<<s1 <<endl;
                    counter++; //temp used.
                }
				if(!isTemp(s2)){
                    outFile << "\t%gokberki"<<counter <<" = load i32* %"<<s2 <<endl;
                    counter++; //temp used.
                }


                if(temp == "-") { //if operator is -, make subtraction.
                    outFile << "\t%gokberki" << counter << " = sub i32 %gokberki" << counter - 1 << ", %gokberki" << counter - 2 << endl;
                }
                else if(temp == "+"){ //if operator is +, make addition.
                    outFile << "\t%gokberki" << counter << " = add i32 %gokberki" << counter - 1 << ", %gokberki" << counter - 2 << endl;
                }
                else if(temp == "*"){ //if operator is *, make multiplication.
                    outFile << "\t%gokberki" << counter << " = mul i32 %gokberki" << counter - 1 << ", %gokberki" << counter - 2 << endl;
                }
                else if(temp=="/"){ //if operator is /, make division.
                    outFile << "\t%gokberki" << counter << " = sdiv i32 %gokberki" << counter - 1 << ", %gokberki" << counter - 2 << endl;
                }
                counter++; //temp used.
            } 

            //if one of them is variable and the other is an integer,
            else if(var1 && !var2){

            	//if it is not temp already, then load it to a new temp.
                if(!isTemp(s1)){
                    outFile << "\t%gokberki"<<counter <<" = load i32* %"<<s1 <<endl;
                    counter++; //temp used.
                }

                if(temp == "-") { //if operator is -, make subtraction.
                    outFile << "\t%gokberki" << counter << " = sub i32 " << s2 << ", %gokberki" << counter - 1 << endl;
                }
                else if(temp == "+"){ //if operator is +, make addition.
                    outFile << "\t%gokberki" << counter << " = add i32 " << s2 << ", %gokberki" << counter - 1 << endl;
                }
                else if(temp == "*"){ //if operator is *, make multiplication.
                    outFile << "\t%gokberki" << counter << " = mul i32 " << s2 << ", %gokberki" << counter - 1 << endl;
                }
                else if(temp=="/"){ //if operator is /, make division.
                    outFile << "\t%gokberki" << counter << " = sdiv i32 " << s2 << ", %gokberki" << counter - 1 << endl;
                }
              
                counter++; //temp used.
            } 

            //if one of them is variable and the other is an integer,
            else if(!var1 && var2){

            	//if it is not temp already, then load it to a new temp.
                if(!isTemp(s2)){
                    outFile << "\t%gokberki"<<counter <<" = load i32* %"<<s2 <<endl;
                    counter++; //temp used.
                }

                if(temp == "-") { //if operator is -, make subtraction.
                    outFile << "\t%gokberki" << counter << " = sub i32 %gokberki" << counter - 1 << ", " << s1 << endl;
                }
                else if(temp == "+"){ //if operator is +, make addition.
                    outFile << "\t%gokberki" << counter << " = add i32 %gokberki" << counter - 1 << ", " << s1 << endl;
                }
                else if(temp == "*"){ //if operator is *, make multiplication.
                    outFile << "\t%gokberki" << counter << " = mul i32 %gokberki" << counter - 1 << ", " << s1 << endl;
                }
                else if(temp=="/"){ //if operator is /, make division.
                    outFile << "\t%gokberki" << counter << " = sdiv i32 %gokberki" << counter - 1 << ", " << s1 << endl;
                }
               
                counter++; //temp used.
            }

            //if they are both integers,
            else {

                if(temp == "-") { //if operator is -, make subtraction.
                    outFile << "\t%gokberki" << counter << " = sub i32 " << s2 << ", " << s1 << endl;
                }
                else if(temp == "+"){ //if operator is +, make addition.
                    outFile << "\t%gokberki" << counter << " = add i32 " << s2 << ", " << s1 << endl;
                }
                else if(temp == "*"){ //if operator is *, make multiplication.
                    outFile << "\t%gokberki" << counter << " = mul i32 " << s2 << ", " << s1 << endl;
                }
                else if(temp=="/"){ //if operator is /, make division.
                    outFile << "\t%gokberki" << counter << " = sdiv i32 " << s2 << ", " << s1 << endl;
                }
                
                counter++; //temp used.


            }

            st.push("gokberki"+to_string(counter-1)); //push the result of the operands back to the operands stack.
        }

        if(fromAssignment){ //if expression an assignment expression,

            if(q.empty()){ //if there are no elements left in the postfix notation.

                string rhs = st.top(); st.pop(); //last element in the operands stack.

                if(!isTemp(rhs) && !isVariable(rhs)){ //if it is not temp and not a variable,

                	//assigns the value to necessary variable and updates that variable's value on the varmap.
                    outFile << "\tstore i32 " <<rhs <<", i32* %"<<variable_name<<endl;
                    vars[variable_name] = stoi(rhs);

                }
                else if(!isTemp(rhs) && isVariable(rhs)){ //if it is not temp but a variable,
                        
                	//if it is not declared, create it.
                   if(vars.count(rhs)==0){
                        outFile << "\t%"<<rhs<<" = alloca i32"<<endl;
                   	    outFile << "\tstore i32 0, i32* %"<<rhs<<endl;
                        vars.insert(pair<string,int>(rhs,0));
                    }

                    //loads it to a temp, then assigns it to necessary variable.
                    outFile << "\t%gokberki"<<counter<< " = load i32* %"<<rhs <<endl;
                    outFile << "\tstore i32 %gokberki" <<counter <<", i32* %"<<variable_name<<endl;
                    counter++;//temp used.
                }
                else{ //if it is temp,

                	//stores it to necessary variable.
                    outFile << "\tstore i32 %gokberki" <<counter-1 <<", i32* %"<<variable_name<<endl;
                }
            }
        }
        else { //if expression is not an assignment,
            if(q.empty()){ //if there are no elements left in postfix notation.

                string rhs = st.top(); st.pop(); //last element in the operands stack.

                if(!isTemp(rhs) && isVariable(rhs)){ //if it is not temp but a variable,
                    
                	//if it is not declared, create it.
                    if(vars.count(rhs) == 0){
                        outFile << "\t%"<<rhs<<" = alloca i32"<<endl;
                        outFile << "\tstore i32 0, i32* %"<<rhs<<endl;
                        vars.insert(pair<string,int>(rhs,0));
                        
                    }

                    //loads it to a temp.
                    outFile << "\t%gokberki" <<counter <<" = load i32* %"<<rhs<<endl;
                    counter++; //temp used.
                }
                else if( !isVariable(rhs) ){ //if it is not a variable,

                	outFile << "\t%gokberki" << counter << " = add i32 " << rhs << ", " << 0 << endl; //stores it in a new temp.
                	counter++; //temp used.
                }
                else if(isTemp(rhs)){ //if it is already a temp,

                	outFile << "\t%gokberki"<<counter<<" = add i32 %"<<rhs<<", 0"<<endl; //stores it in a new temp.
                	counter++; //temp used.

                }
            }
        }

    }
}

int syntaxCheck(ifstream &inFile2) { //checks the syntax of given input line by line

    string line ;               //string to store and check current line
    

    int count=-1;               //line counter

    vector<char> operators { '+', '-', '/', '*', '(', ')', '{', '}', '=' , ',' };   //vector of valid operators

    //some vectors which are not used in programming just as a reminder of valid syntax of different constructs
    vector<string> ifSyntax {"if", "(","expr",")","{"};
    vector<string> whileSyntax {"while", "(","expression",")","{"};
    vector<string> assignSyntax {"variable","=","expression"};
    vector<string> printSyntax {"print","(","expression",")"};

    stack<char> curlyBracketCheck;  //stack to check curly bracket numbers

    while( getline(inFile2,line) ){ // get line by line and make the necessary controls

        count++;    //increment the line count as we process a new line
        string str = "";    //a string variable which will contain tokenized version of input line

        stack<char> paranthesisCheck;   //this one is for checking regular paranthesis in a single line

        int index_of_comment = line.find("#");  //if there is comment in a line ignore them

        if(index_of_comment != -1){             

            line = line.substr(0, index_of_comment);    //line until comment starts
        }

        if(line.length()==0){   //if line is empty move on
            continue;
        }

        for(int i=0;i<line.length();i++){   //iterate each character in the line

            char c= line.at(i);         //current character

            //if character is valid append it to tokenized string str
            if( (c >= 'a' && c <='z') || (c >= 'A' && c <='Z') || (c >= '0' &&c <= '9') || c==' ' || c=='\t' ){

                str = str + c;
            }   
            else if( find(operators.begin(),operators.end(), c) != operators.end() ){  

                //if character is an operator put blank spaces before and after it so we can tokenize
                str= str + " " + c + " ";

                if(c == '('){   //if it is an opening paranthesis push it to stack

                    paranthesisCheck.push('(');
                }
                else if(c == ')'){  //if it is a closing paranthesis pop it from stack if stack is empty then error

                    if(paranthesisCheck.empty()){
                        return count;
                    } else {
                        paranthesisCheck.pop();
                    }
                }
                else if(c=='{'){    //if current char is opening curly brackets push it to curly bracket stack
                    
                    curlyBracketCheck.push('{');

                    if(curlyBracketCheck.size()==2){    //since we dont have nested ifs and whiles 2 opening paranthesis cannot come one after another
                        return count;   
                    }
                }
                else if(c=='}'){       //if current char is closing curly brackets pop it from stack if stack is empty then error

                    if(curlyBracketCheck.empty()){
                        return count;
                    }
                    else {
                        curlyBracketCheck.pop();
                    }
                }
            }else if(c=='\n' || c=='\r'){   //some other checks to prevent errors

                str += " ";
            }
            else {              //if current character is not of one the above then error
                return count;
            }
        }

        if(!commacheck(str)){   //this method checks if comma and choose numbers overlap will be explained later
            return count;
        } 

        if(!paranthesisCheck.empty()){ // if there is an unmatched paranthesis then error
            return count;
        }

        //some string variables to be used in checking construct synax
        string firstToken="";
        string nextTokens="";
        string expression="";

        while(str.find("choose (") != string::npos){ //if there is a choose statement
            
            str = chooseSyntaxChecker(str);     //check the syntax if choose if its valid return the same line with 1 instead of choose statement

            if(str.length()==0){                //above method returns an empty string if syntax is invalid so error 
                return count;
            }
        }

        stringstream ss;                        //stringstream to tokenize the line
        ss << str;                              //tokenize the string str

        if(!(ss >> firstToken)){            //no tokens means nothing to check move on          
            continue;
        }

        if(firstToken == "while" || firstToken == "if"){          //while and if statemens have the same syntax

            bool variableExpect=true;       //boolean to indicates if we expect next token to be variable or not
            ss >> nextTokens;               //get the next token
            int balance =0;                 //to check the paranthesis of an expression in while body

            if(nextTokens=="("){            //next token should be opening paranthesis

                balance++;

                while(balance!=0 && ss>>nextTokens){    //get the expression until we reach the balance

                    if( nextTokens!="+" && nextTokens!="*"&& nextTokens!="-" && nextTokens!="/" && variableExpect && nextTokens!="(" && nextTokens!=")"){
                        
                        //if current token is not an operator ie a variable then next token should not be a variable
                        variableExpect=false;
                    }
                    else if( (nextTokens == "+" || nextTokens == "*" || nextTokens == "/" || nextTokens == "-" ) && !variableExpect){

                        //if current token is an arithmetic operator and we dont expect any variables then next token should be a variable
                        variableExpect=true;
                    }
                    else if( (nextTokens!="/" && nextTokens!="-" && nextTokens!="*" && nextTokens!="+" && nextTokens!="(" && nextTokens!=")") && !variableExpect ){

                        //if current token is an unexpected variable then 2 variables come one ofter another which is an error
                        return count;
                    }
                    if(nextTokens=="("){    //if the token is paranthesis update balance

                        balance++;
                    }
                    else if(nextTokens==")"){    //if the token is paranthesis update balance

                        balance--;

                        if(balance==0){         //when we find the closing paranthesis of while statement break
                            break;
                        }
                    }

                    expression += nextTokens;   //append tokens to expression string to check its syntax 
                }
            }else {     //if next token after "while" is not paranthesis then error
                return count;
            }

            ss >> nextTokens;       
            if(nextTokens=="{"){        //next token after we find the expression should be curly bracket
               
                while(ss>>nextTokens){  //if there is any token afterwards error
                    return count;
                }
            }
            else{                       //if expression is not followed by curly bracket error
                return count;
            }
            
            if(!syntaxPostFix(inToPost(expression))){   //checks the syntax of expression found above
                return count;
            }

        }
        else if(firstToken == "print"){         //if line is a print statement

            bool variableExpect=true;

            ss >> nextTokens;
            int balance =0;                 //to balance paranthesis so that we can get expression inside

            if(nextTokens=="("){            //next token should be opening paranthesis
                balance++;

                while(balance!=0 && ss>>nextTokens){    //same method as used above to get the expression
                    
                    if( nextTokens!="+" && nextTokens!="*"&& nextTokens!="-" && nextTokens!="/" && variableExpect && nextTokens!="(" && nextTokens!=")"){

                        variableExpect=false;
                    }
                    else if( (nextTokens == "+" || nextTokens == "*" || nextTokens == "/" || nextTokens == "-" ) && !variableExpect){
                        
                        variableExpect=true;
                    }
                    else if( (nextTokens!="/" && nextTokens!="-" && nextTokens!="*" && nextTokens!="+" && nextTokens!="(" && nextTokens!=")") && !variableExpect ){
                       
                        return count;
                    }

                    if(nextTokens=="("){        //update balance when token is paranthesis
                        balance++;
                    }
                    else if(nextTokens==")"){

                        balance--;
                        if(balance==0){
                            break;
                        }
                    }
                    expression += nextTokens;

                }
            }
            else {  //if first token is not opening paranthesis then error 
                return count;
            }

            if(nextTokens==")"){    //after expression should be closing paranthesis
                
                while(ss>>nextTokens){  //if any token comes after closing paranthesis then error
                    return count;
                }
            }   
            else{                   //if next token is not closing paranthesis then error
                return count;
            }

            if(!syntaxPostFix(inToPost(expression))){   //check if the expression inside is valid or not
                return count;
            }
        }

        else if(str.find("=")!= -1){   //assignment statement condition

            if(!isVariable(firstToken)){    //if firstToken is not a variable then error

                return count;
            }else {

            	if(isVariable(firstToken)&&vars.count(firstToken)==0){ 
                    //if left hand side did not show up before add it to variable map which will be used in main to allocate space to variables
                    vars.insert(pair<string,int>(firstToken,0));
                }
            }

            ss >> nextTokens;       //ge the next token

            if(nextTokens!= "="){   //next token should be assignment operator
                return count;
            }

            bool variableExpect=true;

            while( ss>>nextTokens){     //same method used in while and if statements

                if( nextTokens!="+" && nextTokens!="*"&& nextTokens!="-" && nextTokens!="/" && variableExpect && nextTokens!="(" && nextTokens!=")"){

                    variableExpect=false;
                }

                else if( (nextTokens == "+" || nextTokens == "*" || nextTokens == "/" || nextTokens == "-" ) && !variableExpect){
                    
                    variableExpect=true;
                }
                else if( (nextTokens!="/" && nextTokens!="-" && nextTokens!="*" && nextTokens!="+" && nextTokens!="(" && nextTokens!=")") && !variableExpect ){
                    
                    return count;
                }

                expression += nextTokens;
            }

            if(!syntaxPostFix(inToPost(expression))){   //check if the expression is valid or not
                return count;
            }
        }
        

        else if(firstToken=="}"){   //if first toke is closing curly bracket no token should follow it
            
            if(ss>>nextTokens){
                return count;
            }
        }

        else {                      //if first token is not one of the above then error
            return count;
        }
    }

    //if there are missing closing brackets then error
    if(!curlyBracketCheck.empty()){
        return count;
    }

    return -1;          //if there is no syntax error returns -1 
}

string chooseSyntaxChecker(string str) {    //checks the syntax of choose statement in a line

	chooseCount++;             //increment the number of choose statements

    string final = "";          //string to be returned when checking is done

    string temp = "";           //this will help us keep track of the strings

    stringstream ss;            //stringstream to read token by token from given string
    ss << str;

    string next = "";           //string to store tokens
    ss >> next;

    while(next!= "choose"){    //until finding choose dont modify the string

        final += next;
        final += " ";

        temp += next;
        temp += " ";

        ss >> next; //here we get the choose
       
    }

    temp += next;       
    temp += " ";

    ss>>next;                   //here we get the opening paranthesis
    temp += next;
    temp += " ";

    if(next !="("){             //if token is not opening paranthesis then error
        return "";
    }

    //this part is about finding the first expression in a choose statement
    string expr1 = "";      //store the expression
    ss >> next;

    while(next!=","){       //until finding a comma append the tokens to first expression
        
        if(next=="choose"){ //if there is a nested choose statement 

            string rest = "choose ";        //this will store the rest of the string after "choose"
            
            while(ss>>next){                //append rest of the line to string rest
                rest+= next;
                rest+= " ";
            }

            rest = chooseSyntaxChecker(rest);   //recursive call to the same function

            if(rest.length()==0){               //if it returns an empty string it means syntax of choose is wrong
                return "";
            }

            temp = temp + " " + rest;           //after leaving recursion append the string contains before the choose and after choose statements together
           
            final = chooseSyntaxChecker(temp);  //one more recursive call to check if there is any choose in the first expression
           
            return final;
        }
        temp += next;           //update the temp string 
        temp += " ";

        expr1+= next;           //update expression 1
        ss>>next;
    }
    
    if(!syntaxPostFix(inToPost(expr1))){    //check the syntax of expression 1
        return "";
    }

    //same steps as finding expression 1
    string expr2="";
   
    temp += next;
    temp += " ";
    ss>>next;
     
    while(next!=","){
        
        if(next=="choose"){
            string rest = "choose ";
            
            while(ss>>next){

                rest+= next;
                rest+= " ";
            }

            rest = chooseSyntaxChecker(rest);
            if(rest.length()==0){
                return "";
            }

            temp = temp + " " + rest;
            
            final = chooseSyntaxChecker(temp);
            return final;
        }

        temp += next;
        temp += " ";

        expr2+= next;
        ss>>next;
    }

    if(!syntaxPostFix(inToPost(expr2))){    //check the syntax of expression 2
        return "";
    }

    //same steps as finding expression 1 and 2
    string expr3 = "";
    
    temp += next;
    temp += " ";
    ss>>next;
    
    while(next!=","){

        if(next=="choose"){
            string rest = "choose ";
           
            while(ss>>next){
                rest+= next;
                rest+= " ";
            }

            rest = chooseSyntaxChecker(rest);
            if(rest.length()==0){
                return "";
            }

            temp = temp + " " + rest;

            final = chooseSyntaxChecker(temp);
            return final;
        }
        temp += next;
        temp += " ";

        expr3+= next;
        ss>>next;
    }

    if(!syntaxPostFix(inToPost(expr3))){    //check the syntax of expression 3
        return "";
    }

    //Finding expression 4 only difference is we need to watch out for closing paranhtesis of choose 
    string expr4 = "";
    int paranthCounter = 0;         //this will help us control if we reach the paranthesis or not

    temp += next;
    temp += " ";
    while(paranthCounter!=-1){      //until reaching closing paranthesis
        
        ss>>next;
        
        if(next=="choose"){
            string rest = "choose ";
           
            while(ss>>next){
                rest+= next;
                rest+= " ";
            }

            rest = chooseSyntaxChecker(rest);
            if(rest.length()==0){
                return "";
            }

            temp = temp + " " + rest;
            
            final = chooseSyntaxChecker(temp);
            return final;
        }

        if(next=="("){          //if we find an opening paranthesis increment paranthCounter
            paranthCounter++;

        }else if(next==")"){    //decrement when we find closing paranthesis if reach -1 break
            paranthCounter--;
            if(paranthCounter==-1){
                break;
            }
        }

        temp += next;
        temp += " ";

        expr4+= next;

    }
    
    if(!syntaxPostFix(inToPost(expr4))){        //check the syntax of expression 4
        return "";
    }

    //when we reach the most inner choose we replace the whole statement with 1 
    //since we are only checking syntax at this step it doesnt affect the result
    final+= "1 ";           

    while(ss>>next){        //append the rest of string 
        final += next;
        final += " ";
    }

    return final;           //this string in the end has contains a string in which all the choose statements replaced with 1s to be further checked 
}

int choose(string text){ //returns the temporary which hold the value of choose.

	//if there is an inner choose,
    int inner_choose = text.find("choose("); 
    if(inner_choose != -1){

    	//finds the inner choose.
        string examine = text.substr(inner_choose+7);
        int balance_check = 1;
        int i;
        for(i=0; i<examine.length(); i++){
            if(examine.at(i) == '(') balance_check++;
            else if(examine.at(i) == ')') balance_check--;
            if(balance_check == 0) break;
        }
		string inside_of_parenthesis = examine.substr(0,i); //inside of the inner choose.

		//replaces choose with the its temporary.
        int replacement = choose(inside_of_parenthesis);
        string replacementt = "datdiri3datdat1daaatdat" + to_string(replacement);
        text.replace(text.begin()+inner_choose, text.begin()+inner_choose+inside_of_parenthesis.length()+8, replacementt.begin(), replacementt.end());

    }
    
    //choose function outputter to llvm.
    outFile << "\tbr label %mychoose"<<mychoose_counter<<"\n\nmychoose"<<mychoose_counter<<":" << endl;

    //finds the first expression out of four from the choose parameters.
    int index_of_first_comma = text.find(",");
    string expr_1 = text.substr(0, index_of_first_comma);

    //evaluates first expression.
    string postfix_1 = inToPost(expr_1);
    queue<string> q1 = evaluate(postfix_1);
    stack<string> st1;
    expressionToIR(q1, st1, vars, false, "");

    //if condition
    //if the result of the first expression is equal to 0, execute second expression. else check the other condition.
    outFile << "\t%gokberki" <<counter <<" = icmp eq i32 %gokberki"<<counter-1<<" , 0" <<endl;
    int datdatdat = counter-1; //datdatdat means temp to compare inside if statements.
    counter++; //temp used.
    outFile << "\tbr i1 %gokberki"<< counter-1<<", label %myexpr2"<<mychoose_counter<<", label \%others"<<mychoose_counter <<endl;
    
    //if body
    //if first expresion is equal to 0,
    outFile << "\nmyexpr2"<<mychoose_counter<<":" << endl;

    //finds the second expression out of four from the choose parameters.
    string rest = text.substr(index_of_first_comma + 1);
    int index_of_second_comma = rest.find(",");
    string expr_2 = rest.substr(0, index_of_second_comma);

    //evaluates second expression.
    string postfix_2 = inToPost(expr_2);
    queue<string> q2 = evaluate(postfix_2);
    stack<string> st2;
    expressionToIR(q2, st2, vars, false, "");

    //stores the result of second expression in register.
	outFile << "\tstore i32 %gokberki"<<counter-1<< ", i32* \%datdiri3datdat1daaatdat"<<mychoose_counter<<endl;

	//exits the choose function.
	outFile << "\tbr label \%end"<<mychoose_counter <<endl;
    
    //if condition
    //if the result of the first expression is greater than 0, execute third expression. else fourth.
    outFile << "\nothers"<<mychoose_counter<<":" <<endl;
	outFile << "\t%gokberki" <<counter<<" = icmp sgt i32 %gokberki"<< datdatdat << ", 0" <<endl;
	counter++; //temp used.
    outFile << "\tbr i1 %gokberki"<< counter-1<<", label %myexpr3"<<mychoose_counter<<", label %myexpr4"<<mychoose_counter <<endl;
    
    //if body
    //if first expression is greater than 0,
    outFile << "\nmyexpr3"<<mychoose_counter<<":" << endl;

    //finds the third expression out of four from the choose parameters.
    rest = rest.substr(index_of_second_comma + 1);
    int index_of_third_comma = rest.find(",");
    string expr_3 = rest.substr(0, index_of_third_comma);

    //evaluates third expression.
    string postfix_3 = inToPost(expr_3);
    queue<string> q3 = evaluate(postfix_3);
    stack<string> st3;
    expressionToIR(q3, st3, vars, false, "");

    //stores the result of third expression in register.
	outFile << "\tstore i32 %gokberki"<<counter-1<< ", i32* \%datdiri3datdat1daaatdat"<<mychoose_counter<<endl;

	//exits the choose function.
	outFile << "\tbr label \%end"<<mychoose_counter << endl;
    
    //if body
    //if first expression is less than 0,
    outFile << "\nmyexpr4"<<mychoose_counter<<":" << endl;

    //finds the fourth expression out of four from the choose parameters.
    string expr_4 = rest.substr(index_of_third_comma + 1);

    //evaluates fourth expression.
    string postfix_4 = inToPost(expr_4);
    queue<string> q4 = evaluate(postfix_4);
    stack<string> st4;
    expressionToIR(q4, st4, vars, false, "");

	//stores the result of fourth expression in register.
	outFile << "\tstore i32 %gokberki"<<counter-1<< ", i32* \%datdiri3datdat1daaatdat"<<mychoose_counter<<endl;

	//exits the choose function.
	outFile << "\tbr label \%end"<<mychoose_counter << endl;
    
    //end of the choose.
    outFile << "\nend"<<mychoose_counter<<":" << endl;
    mychoose_counter++; //choose encountered.
    
    return mychoose_counter-1; //returns the number of choose.

}

bool commacheck(string str){ //checks whether the choose has consistent number of commas and "choose".
	stringstream ss;
	ss << str;
	string hubhub;
	int comma = 0;
	while(ss >> hubhub){

		//there must be exactly three commas for each "choose".
		if(hubhub == "choose") comma+=3;
		else if(hubhub == ",") comma--;
	}
	if(comma != 0)return false;
	else return true;
}
