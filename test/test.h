#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "MALtypes.h"
#include "Environment.h"

#include "step0_repl.h"
#include "step1_read_print.h"
#include "step2_eval.h"
#include "step3_env.h"
#include "step4_if_fn_do.h"
#include "step5_tco.h"
#include "step6_file.h"
#include "step7_quote.h"
#include "step8_macros.h"
#include "step9_try.h"
#include "stepA_mal.h"


using std::ifstream; using std::ofstream; using std::string;



class FileOutputTester{
	//paths
	string testFiles_folder_path = "test/custom_tests/";
	string original_testFiles_folder_path = "test/original_tests/";
	string logFiles_path = "log/";
	string base_path = "./";
	vector<string> test_files = {"step0_repl.mal","step1_read_print.mal", "step2_eval.mal", "step3_env.mal", "step4_if_fn_do.mal", "step5_tco.mal",
		"step6_file.mal", "step7_quote.mal", "step8_macros.mal", "step9_try.mal", "stepA_mal.mal"};

	string verbose_output_file_name = "verbose_test_output.txt";
	string compact_output_file_name = "compact_test_output.txt";
	string failed_tests_output_file_name = "failed_tests.txt";

	//variables to track the results of the tests
	bool results_need_printing = false;//set this when at least 1 test completed since the last time results were printed
	bool last_test_failed = true;//set this when the last test failed

	int line_number = 0;//current line number in the test file

	int total_tests = 0;//total number of tests in the test file
	int tests_in_block = 0;//total number of tests in current block
	int total_tests_failed = 0;//total number of tests that failed
	int block_tests_failed = 0;//total number of tests that passed

	void log(string& line);

	void log_failed_test(const string& line);

	string finish_test_block();
	string eofSummary();

	//variables to store the results of the current test
	string input;
	string actual_result;
	string actual_cout;
	string expected_result;
	string expected_cout;

	string test_result;


	//filestreams
	ifstream testFile; 
	ofstream verbose_output_file;
	ofstream compact_output_file;
	ofstream failed_tests_output_file;

	std::stringstream sbuffer;//stringbuffer to redirect cout to

	std::streambuf* coutbuf; //save old buf

public:
	FileOutputTester();
	~FileOutputTester();

	//opens logfiles and redirects the cout stream to a stringbuffer
	bool openLogFiles();
	//closes logfiles, resores the cout stream, and prints the results of the tests to the console
	bool closeLogFiles();
	//opens the test file with the given name
	bool openTestFile(string filename);
	//closes the test file 
	bool closeTestFile();

	void runAllTests();
	//runs all tests up to and including stage, then runs the custom test file if a path is provided
	void runTestsUpTo(size_t stage, string custom_path = "");
	//runs a testfile specified by stage, unless a custom_path is provided
	void runSingleStepTests(size_t stage, string custom_path = "");


	//reset all variable used to track tests/results, 
	void resetTestVariables();


	void read_title(const string& line);
	bool read_output(const string& line);
	bool read_cout(const string& line);
	bool read_input(const string& line, size_t stage);

	void write_output_files(string line,bool add_endl = true);
	void write_output_file_verbose(string line, bool add_endl = true);
	void write_output_file_compact(string line, bool add_endl = true);
	void write_failed_tests_file(string line, bool add_endl = true);

	bool compare_results(string actual, string expected);
	bool compare_results_regex(string actual, string expected);
};


