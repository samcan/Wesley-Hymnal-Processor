// Wesley Hymnal Processor's main program file
// Copyright 2012 by Sam Cantrell

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <iterator>

using namespace std;
using namespace boost::algorithm;
namespace po = boost::program_options;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// FUNCTIONS
// Check to see if given file exists.
bool fileExists(const string fileName);
// Open hymn file and process it, adding it to the output,
// and then closing it.
int processHymn(string hymnFileName, string outputFileName);
// Open output, insert page break, and then close output
// file.
int insertNewPage(string outputFileName);

// GLOBAL VARIABLES
bool force = false;
bool quiet = false;

// PROGRAM BEGINS HERE
int main(int argc, char* argv[])
{
	// VARIABLE DECLARATION
	// Variable for the input hymnal file name
	string hymnalFileName;
	// Variable for the output ABC file name
	string outputFileName;
	// File handle for input file
	ifstream hymnalData;
	// Variable for grabbing a line of data from the hymnal input file
	// to process
	string lineOfData;

	// Parse command line arguments
	// TODO: Write error handling code for when
	// output file option is given without file name
	po::options_description desc("Usage");
	desc.add_options()
		("version,v", "Print version")
		("force,f", "Force overwrite of output file if existing")
		("quiet", "Quiet output from program")
		("help", "Show help message")
		("input-file", po::value<string>(), "Wesley HYmnal input file")
		("output-file,o", po::value<string>(), "ABC output file")
	;

	po::positional_options_description p;
	p.add("input-file", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || argc <= 1)
	{
		// Display usage options
		cout << desc << endl;
		return EXIT_SUCCESS;
	}

	if (vm.count("version"))
	{
		// Eventually print version string here
		cout << "Wesley" << endl;
		return EXIT_SUCCESS;
	}

	if (vm.count("quiet"))
	{
		// Quiet output
		quiet = true;
	}

	if (vm.count("force"))
	{
		// Force overwrite of output file if existing
		force = true;
	}

	if (vm.count("input-file"))
	{
		hymnalFileName = vm["input-file"].as<string>();
	}

	if (vm.count("output-file"))
	{
		outputFileName = vm["output-file"].as<string>();
	}
	else
	{
		// No output file provided, so give it Out.abc
		// for now
		outputFileName = "Out.abc";
	}

	// Attempt to open hymnal data file
	hymnalData.open(hymnalFileName.c_str());
	if (!hymnalData)
	{
		// File not found or successfully opened, abort.
		cout << "Error: " << hymnalFileName << " not found." << endl;
		return EXIT_FAILURE;
	}

	// Check to see if the file listed for output already
	// exists.
	if (fileExists(outputFileName) == true)
	{
		// File found, check to see if force flag is set
		if (!force)
		{
			// Force flag not set, so abort
			cout << "Error: Output file already exists." << endl;
			return EXIT_FAILURE;
		}
		else
		{
			// Force flag set, so delete file
			remove(outputFileName.c_str());
		}
	}

	// Now begin parsing the file
	// Right now hymns aren't sorted by their numbers, but rather inserted
	// in the order they are listed in the WHY (Wesley HYmnal) file. This
	// means that the user has to order them correctly.
	while(getline(hymnalData, lineOfData))
	{
		// Get each line of data from hymnal file. Right now, that
		// should only be either a %%NEWPAGE code or a hymn file name.

		// Check for %%NEWPAGE code
		if (lineOfData == "%%NEWPAGE")
		{
			if (insertNewPage(outputFileName) != EXIT_SUCCESS)
			{
				// Returned with error condition
				cout << "Error: Unable to open " << outputFileName << endl;
				return EXIT_FAILURE;
			}
		}
		else
		{
			// Must be a hymn file name
			if (processHymn(lineOfData, outputFileName) != EXIT_SUCCESS)
			{
				// There was an error processing this hymn
				return EXIT_FAILURE;
			}
		}
	}

	// Close hymnal input file, as we're done
	hymnalData.close();

	return EXIT_SUCCESS;
}

int processHymn(string hymnFileName, string outputFileName)
{
	// Var for holding data pulled in from file
	string data;

	ifstream hymnData;
	ofstream outputData;

	// Information fields for hymn
	// Would this work better as a struct?
	string title;
	string composer;
	string lyricist;
	string meter;
	string tune;
	string time;

	string category;

	// TODO: Presently scripture is not loaded.
	string scripture;

	string abcData;

	string staffSkip;
	int no = 0;
	float scale = 0.00;
	bool titleTrim = true;
	// End of declaring information fields


	// Open hymn file passed in
	hymnData.open(hymnFileName.c_str());
	if (!hymnData)
	{
		// File not found or opened successfully, abort.
		// We print the filename so that the user may
		// know which specific hymn caused the program
		// failure.
		cout << "Error: " << hymnFileName << " not opened successfully." << endl;
		return EXIT_FAILURE;
	}

	// Notify user that hymn was opened successfully.
	if (!quiet)
	{
		cout << hymnFileName << " opened successfully." << endl;
		cout << "Parsing hymn..." << endl;
	}
	hymnData >> data;
	while (!hymnData.eof())
	{
		if (data == "%%NUMBER")
		{
			hymnData >> no;
		}
		else if (data == "%%TITLE")
		{
			char tempData[99];
			hymnData.getline(tempData, 99, '\n');

			title = tempData;
			trim(title);
		}
		else if (data == "%%COMPOSER")
		{
			char tempData[99];
			hymnData.getline(tempData, 99, '\n');

			composer = tempData;
			trim(composer);
		}
		else if (data == "%%LYRICIST")
		{
			char tempData[99];
			hymnData.getline(tempData, 99, '\n');

			lyricist = tempData;
			trim(lyricist);
		}
		else if (data == "%%METER")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');

			meter = tempData;
			// Check for DOS-style line ending and
			// remove if found.
			if (meter[meter.length() - 1] == '\r')
			{
				meter = meter.substr(0, meter.length() - 1);
			}
		}
		else if (data == "%%TUNE")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');

			tune = tempData;
			// Check for DOS-style line ending and
			// remove if found.
			if (tune[tune.length() - 1] == '\r')
			{
				tune = tune.substr(0, tune.length() - 1);
			}
		}
		else if (data == "%%CATEGORY")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');

			category = tempData;
			trim(category);
		}
		else if (data == "%%TIMESIGNATURE")
		{
			hymnData >> time;
		}
		else if (data == "%%SCALE")
		{
			hymnData >> scale;
		}
		else if (data == "%%STAFFSKIP")
		{
			hymnData >> staffSkip;
		}
		else if (data == "%%NOTITLETRIM")
		{
			// Disable title trim
			titleTrim = false;
		}
		else if (data == "%%BEGIN")
		{
			// This was a little finicky. I wasn't sure how to abort
			// out of the if statement and then the while so that
			// I could run a separate loop to grab all the ABC data,
			// so instead I embedded the loop in here. This is very
			// hacky!
			// TODO: Fix this later!
			while (!hymnData.eof())
			{
				char line[300];
				hymnData.getline(line, 300, '\n');
				if (line != "") {
					abcData = abcData + line + "\n";
				}
			}
		}
		hymnData >> data;
	}

	// Close hymn file
	hymnData.close();

	// Create output
	outputData.open(outputFileName.c_str(), ios::out | ios::app);

	if (!outputData)
	{
		// Output file not successfully opened; abort.
		cout << "Error: Output file not successfully opened." << endl;
		return EXIT_FAILURE;
	}

	// Create format information such as scale and staff skip amounts
	if (scale != 0) {
		outputData << "%%scale " << scale << endl;
	}

	if (staffSkip != "") {
		outputData << "%%staffskip " << staffSkip << endl;
	}

	outputData << "%%titletrim " << (titleTrim ? "true" : "false") << endl;

	// Put in the footer format; TODO: Add header info
	outputData << "%%footer \" 		" << tune << "\\n		" << meter << "\"" << endl;

	// Output the information fields
	outputData << "X: " << no << endl;
	outputData << "T: " << title << endl;
	outputData << "H: " << lyricist << endl;
	outputData << "C: " << composer << endl;
	outputData << "M: " << time;

	// Send the actual ABC data
	outputData << abcData << endl;
	outputData << endl << endl;

	// Close output file
	outputData.close();
	return EXIT_SUCCESS;
}

int insertNewPage(string outputFileName)
{
	// This function inserts a new page command
	ofstream outputData;

	outputData.open(outputFileName.c_str(), ios::out | ios::app);
	if (outputData)
	{
		outputData << endl << endl;
		outputData << "%%newpage" << endl;
		outputData << endl << endl;

		outputData.close();
	}
	else {
		// File not successfully opened
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

bool fileExists(const string fileName)
{
	// Return true if fileName exists, otherwise
	// return false.
	ifstream ifile(fileName.c_str());
	return ifile;
}
