#include "test.h"

#include <regex>
#include <exception>
#include <bitset>

#include "repl.h"
#include "myExceptions.h"

using std::cout; using std::cerr; using std::cin; using std::endl;


FileOutputTester::FileOutputTester(){
	if(openLogFiles())
		std::cerr << "opened log files!\n";
	else
		std::cerr << "failed to open log files!\n";
};

FileOutputTester::~FileOutputTester(){
	if(!closeLogFiles())
		cout << "failed to close log files!\n";
	else
		cout << "closed all files!\n";
}

bool FileOutputTester::openLogFiles(){
	//open verbose output file
	verbose_output_file.open(logFiles_path + verbose_output_file_name);
	compact_output_file.open(logFiles_path + compact_output_file_name);
	failed_tests_output_file.open(logFiles_path + failed_tests_output_file_name);

	cout << "opened output files\n";

	//redirect cout to file
	try{
		coutbuf = cout.rdbuf(); //save old buf
		cout.rdbuf(sbuffer.rdbuf()); //redirect std::cout to a stringsteam that acts as a queue

		std::cerr << "redirected cout\n";
	} catch(std::ofstream::failure e){
		std::cerr << "Exception opening/reading file\n";
	} catch(std::exception e){
		std::cerr << "Exception opening/reading file\n";
	}
	return 	verbose_output_file.is_open() and compact_output_file.is_open() and failed_tests_output_file.is_open();
}

bool FileOutputTester::closeLogFiles(){
	write_output_files(test_result);

	verbose_output_file.close();
	compact_output_file.close();

	failed_tests_output_file.close();
	//reset to standard cout
	std::cout.rdbuf(coutbuf);

	std::cout << test_result << endl;

	return  !verbose_output_file.is_open() and !compact_output_file.is_open() and !failed_tests_output_file.is_open();
}

bool FileOutputTester::openTestFile(string filename){
	testFile.open(testFiles_folder_path + filename);
	if(!testFile.is_open())
		testFile.open(filename);
	return testFile.is_open();
}

bool FileOutputTester::closeTestFile(){
	testFile.close();
	return !testFile.is_open();
}




void FileOutputTester::runAllTests(){
	runTestsUpTo(test_files.size());
}

void FileOutputTester::runTestsUpTo(size_t stage, string custom_path){
	if(stage >= test_files.size())
		stage = test_files.size() - 1; //if stage is too large, run all tests
	for(int i = 0; i <= stage; i++){
		runSingleStepTests(i);
		resetTestVariables();
	}
	//if provided, run the custom test file (path)
	if(custom_path != "")
		runSingleStepTests(stage, custom_path);
}

void FileOutputTester::runSingleStepTests(size_t stage, string custom_path){
	bool is_custom_test_file = custom_path != "";
	string path = is_custom_test_file ? custom_path : test_files[stage];
	if(!openTestFile(path)){
		std::cerr << "failed to open test file " << path << "\n";
		write_output_files("failed to open test file " + path);
		return;
	}


	write_output_files("############################################################################################################");
	write_output_files("############################################################################################################");
	write_output_files("File: " + path);
	write_output_files("############################################################################################################");
	write_output_files("############################################################################################################");

	write_failed_tests_file("############################################################################################################");
	write_failed_tests_file("File: " + path);
	write_failed_tests_file("############################################################################################################");

	//pretend we tried to read a file passed in as an argument when running the program
	if(stage >= 6)
		read_input("(def! *ARGV* ())", stage);

	string line;
	while(getline(testFile, line)){
		++line_number;
		if(!line.empty()){
			string startofline = line.substr(0, 2);

			if(startofline == ";;")							read_title(line);
			else if(startofline == ";>")					read_title(line);//TODO handle deferrables and soft fails
			else if(startofline == ";=")					read_output(line);
			else if(startofline == ";/")					read_cout(line);
			else											read_input(line, stage);
		}
		log(line);
	}

	test_result += is_custom_test_file ? "Custom(" + std::to_string(stage) + ")" : path.substr(0, 5);
	if(!is_custom_test_file) test_result += "\t";
	test_result += "\t->\t\t";
	test_result += (total_tests_failed == 0 ? ">>>>> ALL TESTS PASSED <<<<<" : (">>>>> " + std::to_string(total_tests_failed) + " TESTS FAILED <<<<<"));
	test_result += "\n";

	if(!closeTestFile()) std::cerr << "failed to close test file" << path << "\n";
}



//called for each line read from the test file
void FileOutputTester::log(string& line){

	if(line.empty()) return;
	else if(line.substr(0, 3) == ";;;")
		write_output_file_verbose(std::to_string(this->line_number) + line);
	//title
	else if(line.substr(0, 2) == ";;"){
		//title terminates block
		if(tests_in_block != 0){
			//print block summary to verbose and compact
			write_output_files(finish_test_block());

			//reset block counters
			tests_in_block = 0;
			block_tests_failed = 0;
		}
		//eof summary
		if(line.substr(0, 14) == ";; END OF STEP") //print the eof summary instead of the line/title
			write_output_files(eofSummary());
		else{
			//print title 
			write_output_file_verbose(std::to_string(this->line_number) + " ", false);
			write_output_files(line);
		}
	}
	//not a title or empty line or comment
	else{
		write_output_file_verbose(std::to_string(this->line_number) + " ", false);
		write_output_file_verbose(line);

		if(results_need_printing){ //print result for a single test
			if(last_test_failed)
				write_output_file_verbose("\t\t-> failed");
			else
				write_output_file_verbose("\t\t-> passed");
			results_need_printing = false;
		}
	}
}

void FileOutputTester::log_failed_test(const string& line){
	write_failed_tests_file("line " + std::to_string(this->line_number) + ": " + line);
	write_failed_tests_file("last input: " + input);
	write_failed_tests_file("expected result: " + expected_result);
	write_failed_tests_file("actual result: " + actual_result);
	write_failed_tests_file("expected cout: " + expected_cout);
	write_failed_tests_file("actual cout: " + actual_cout);
	write_failed_tests_file("----------------------------------------------------------------------------------------------------------------------------");
}



string FileOutputTester::finish_test_block(){
	string block_summary;
	block_summary += "[";
	block_summary += std::to_string(tests_in_block - block_tests_failed);
	block_summary += "/";
	block_summary += std::to_string(tests_in_block);
	block_summary += " passed]";

	return block_summary;
}

string FileOutputTester::eofSummary(){
	string eof_summary;
	eof_summary += "*******************\n";
	eof_summary += "end of file summary\n";
	eof_summary += std::to_string(total_tests - total_tests_failed);
	eof_summary += "/";
	eof_summary += std::to_string(total_tests);
	eof_summary += " passed\n";
	eof_summary += "*******************";
	return eof_summary;
}

void FileOutputTester::resetTestVariables(){
	input = "";
	expected_result = "";
	expected_cout = "";
	actual_result = "";
	actual_cout = "";

	sbuffer.str(string());
	sbuffer.clear();

	results_need_printing = false;
	last_test_failed = true;

	line_number = 0;
	total_tests = 0;
	tests_in_block = 0;
	total_tests_failed = 0;
	block_tests_failed = 0;
}

void FileOutputTester::read_title(const string& line){
	// TODO handle deferrables and soft fails
}

bool FileOutputTester::read_output(const string& line){
	expected_result = line.substr(3, string::npos);

	total_tests++;
	tests_in_block++;

	return compare_results(actual_result, expected_result);
}

bool FileOutputTester::read_cout(const string& line){
	expected_cout = line.substr(2, string::npos);

	total_tests++;
	tests_in_block++;

	if(!getline(sbuffer, actual_cout)){
		cerr << "failed to read cout from stringstream\n";
		return false;
	}
	else
		return compare_results(actual_cout, expected_cout);
}

bool FileOutputTester::read_input(const string& line, size_t stage){
	//clear stringstream
	sbuffer.str(string());
	sbuffer.clear();
	input = line;

	//TODO handle errors

	try{
		switch(stage){
			case 0:
				actual_result = step0::rep(line);
				break;
			case 1:
				actual_result = step1::rep(line);
				break;
			case 2:
				actual_result = step2::rep(line);
				break;
			case 3:
				actual_result = step3::rep(line);
				break;
			case 4:
				actual_result = step4::rep(line);
				break;
			case 5:
				actual_result = step5::rep(line);
				break;
			case 6:
				actual_result = step6::rep(line);
				break;
			case 7:
				actual_result = step7::rep(line);
				break;
			case 8:
				actual_result = step8::rep(line);
				break;
			case 9:
				actual_result = step9::rep(line);
				break;
			case 10:
				actual_result = stepA::rep(line);
				break;
			default:
				std::cerr << "invalid stage number\n";
				break;
		}
	} catch(commentException&){//ignore
	} catch(myException& e){
		actual_result = ""; //clear actual_result, as it contains the previous result
		cout << e.what() << "\n";
		return false;
	} catch(std::exception& e){
		actual_result = ""; //clear actual_result, as it contains the previous result
		std::cerr << "unknown error" << e.what() << "\n";
		return false;
	}
	return true;
}



bool FileOutputTester::compare_results(string actual, string expected){
	results_need_printing = true;

	if(actual != expected and !compare_results_regex(actual, expected)){
		//TODO detailed output to a file that only contains failed tests
		++total_tests_failed;
		++block_tests_failed;
		last_test_failed = true;
		log_failed_test("expected: " + expected + " but got: " + actual);
		return false;
	}
	else
		last_test_failed = false;
	return true;

}

bool FileOutputTester::compare_results_regex(string actual, string expected){

	try{
		std::regex expected_regex(expected);
		return std::regex_match(actual, expected_regex);
	} catch(std::regex_error& e){
		write_output_file_verbose("regex error: " + string(e.what()));
		write_output_file_verbose("expected: " + expected);
		write_output_file_verbose("actual: " + actual);
		return false;
	}
	return false;
}



void FileOutputTester::write_output_files(string line, bool add_endl){
	write_output_file_compact(line, add_endl);
	write_output_file_verbose(line, add_endl);
}

void FileOutputTester::write_output_file_verbose(string line, bool add_endl){
	verbose_output_file << line;
	if(add_endl)
		verbose_output_file << "\n";
}

void FileOutputTester::write_output_file_compact(string line, bool add_endl){
	compact_output_file << line;
	if(add_endl)
		compact_output_file << "\n";
}

void FileOutputTester::write_failed_tests_file(string line, bool add_endl){
	failed_tests_output_file << line;
	if(add_endl)
		failed_tests_output_file << "\n";
}

int main(int argc, char** argv){
	// FileOutputTester().runAllTests();
	FileOutputTester().runTestsUpTo(6);
	// FileOutputTester().runSingleStepTests(6);
	return 0;
}