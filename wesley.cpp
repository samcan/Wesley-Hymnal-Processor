// Wesley Hymnal Processor's main program file
// Copyright 2012 by Sam Cantrell

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iterator>
#include <vector>

using namespace std;
using namespace boost::algorithm;
using namespace boost::filesystem;
namespace po = boost::program_options;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// FOR INDEX CREATION
struct hymnEntry {
	int no;
	string title;
};

struct categoryEntry {
	string title;
	vector<hymnEntry> hymnList;
};

// FUNCTIONS
// Check to see if given file exists.
bool fileExists(const string fileName);
// Open hymn file and process it, adding it to the output,
// and then closing it.
int processHymn(const string hymnFileName, const string outputFileName, vector<hymnEntry> *hymnIndex, vector<categoryEntry> *categoryIndex);
// Open output, insert page break, and then close output
// file.
int insertNewPage(string outputFileName);
// Sort index
void sort(vector<hymnEntry> *indexToSort);
void sort(vector<categoryEntry> *indexToSort);
// Bubble sort algorithm
void bubbleSort(vector<hymnEntry> *hymnList, int length);
void bubbleSort(vector<categoryEntry> *list, int length);

// GLOBAL VARIABLES
bool force = false;	// Force overwrite of existing output file
bool quiet = false;	// Quiet output

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
		// No output filename provided, so give it
		// the filename of the input file, but with
		// an ABC extension
		path tempPath(hymnalFileName);
		outputFileName = basename(tempPath);
		outputFileName = outputFileName + ".abc";
	}


	// Create general index
	vector<hymnEntry> hymnIndex;

	// Create overall category index
	vector<categoryEntry> categoryIndex;

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
			if (processHymn(lineOfData, outputFileName, &hymnIndex, &categoryIndex) != EXIT_SUCCESS)
			{
				// There was an error processing this hymn
				return EXIT_FAILURE;
			}
		}
	}

	// Close hymnal input file, as we're done
	hymnalData.close();

	// Sort the general index
	sort(&hymnIndex);

	// Sort the category index
	sort(&categoryIndex);

	// Just to make sure I'm not crazy, let's print out a list of categories
	// I'll comment this code out for now, since it's working...

	if (categoryIndex.empty() != true)
	{
		for (int i = 0; i <= categoryIndex.size() - 1; i++)
		{
			cout << categoryIndex.at(i).title << endl;
			for (int j = 0; j <= categoryIndex.at(i).hymnList.size() - 1; j++)
			{
				cout << categoryIndex.at(i).hymnList.at(j).no << " ";
				cout << categoryIndex.at(i).hymnList.at(j).title << endl;
			}
		}
	}


	cout << endl << endl;
	if (hymnIndex.empty() != true)
	{
		for (int index = 0; index <= hymnIndex.size() - 1; index++)
		{
			cout << hymnIndex.at(index).title << " ";
			cout << hymnIndex.at(index).no << endl;
		}
	}
	return EXIT_SUCCESS;
}

int processHymn(const string hymnFileName, const string outputFileName, vector<hymnEntry> *hymnIndex, vector<categoryEntry> *categoryIndex)
{
	// Enumeration for which side of the page
	// the hymn is on, for the hymn number.
	enum sideOfPage {SIDE_LEFT, SIDE_RIGHT};

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

	string category;

	// TODO: Presently scripture is not loaded.
	string scripture;

	string abcData;

	string staffSkip;
	int no = 0;
	float scale = 0.00;
	bool titleTrim = true;

	// Var for storing which side of the page
	// a hymn is on
	int side = SIDE_RIGHT;
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
			getline(hymnData, title, '\n');
			trim(title);

			hymnEntry tempHymn;
			tempHymn.no = no;
			tempHymn.title = title;

			hymnIndex->push_back(tempHymn);
		}
		else if (data == "%%COMPOSER")
		{
			getline(hymnData, composer, '\n');
			trim(composer);
		}
		else if (data == "%%LYRICIST")
		{
			getline(hymnData, lyricist, '\n');
			trim(lyricist);
		}
		else if (data == "%%METER")
		{
			getline(hymnData, meter, '\n');
			// Check for DOS-style line ending and
			// remove if found.
			if (meter[meter.length() - 1] == '\r')
			{
				meter = meter.substr(0, meter.length() - 1);
			}
		}
		else if (data == "%%TUNE")
		{
			getline(hymnData, tune, '\n');
			// Check for DOS-style line ending and
			// remove if found.
			if (tune[tune.length() - 1] == '\r')
			{
				tune = tune.substr(0, tune.length() - 1);
			}
		}
		else if (data == "%%CATEGORY")
		{
			getline(hymnData, category, '\n');
			trim(category);

			// Check for a blank category, and break out,
			// because we don't want Wesley to add a blank
			// category to the index.
			if (category.empty() == true)
			{
				break;
			}

			// Process category for indexing
			// First check to see if category already
			// exists.
			// TODO: Implement better find algorithm
			int i = 0;
			bool found = false;
			if (categoryIndex->empty() == false)
			{
				for (i = 0; i <= (categoryIndex->size() - 1); i++)
				{
					if (categoryIndex->at(i).title == category)
					{
						found = true;
						break;
					}
				}
			}

			if (found)
			{
				hymnEntry tempHymn;
				tempHymn.no = no;
				tempHymn.title = title;

				categoryIndex->at(i).hymnList.push_back(tempHymn);
			}
			else
			{
				hymnEntry tempHymn;
				tempHymn.no = no;
				tempHymn.title = title;

				categoryEntry tempCategory;
				tempCategory.title = category;
				tempCategory.hymnList.push_back(tempHymn);

				categoryIndex->push_back(tempCategory);
			}
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
		else if (data == "%%LEFT")
		{
			// Hymn number should be on the
			// left side.
			side = SIDE_LEFT;
		}
		else if (data == "%%RIGHT")
		{
			// Hymn number should be on the
			// right side.
			side = SIDE_RIGHT;
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
				string line;
				getline(hymnData, line, '\n');
				trim(line);

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
		outputData << "%%staffsep " << staffSkip << endl;
	}

	outputData << "%%titletrim " << (titleTrim ? "true" : "false") << endl;

	// Put in the header and footer info
	if (side == SIDE_LEFT)
	{
		outputData << "%%titleformat X-1 T0, H-1 C1" << endl;
	}
	else
	{
		outputData << "%%titleformat T0 X1, H-1 C1" << endl;
	}
	outputData << "%%footer \" 		" << tune << "\\n		" << meter << "\"" << endl;

	// Output the information fields
	outputData << "X: " << no << endl;
	outputData << "T: " << title << endl;
	outputData << "H: " << lyricist << endl;
	outputData << "C: " << composer << endl;

	// Send the actual ABC data
	outputData << abcData << endl;

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
		outputData << "%%newpage" << endl;
		outputData << endl;

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

void sort(vector<hymnEntry> *indexToSort)
{
	// First check to make sure vector is not empty (otherwise
	// we'll get an error).
	if (indexToSort->empty() == false)
	{
		// Bubble sort the list
		bubbleSort(indexToSort, indexToSort->size());
	}
}

void sort(vector<categoryEntry> *indexToSort)
{
	// First check to make sure vector is not empty (otherwise
	// we'll get an error).
	if (indexToSort->empty() == false)
	{
		// First bubble sort the overall list (ex. categories,
		// tunes, etc.)
		bubbleSort(indexToSort, indexToSort->size());

		// Now loop through each entry to pass to
		// bubble sort. This sorts the hymns within each
		// index category.
		for (int index = 0; index <= (indexToSort->size() - 1); index++)
		{
			bubbleSort(&(indexToSort->at(index).hymnList), indexToSort->at(index).hymnList.size());
		}
	}
}

void bubbleSort(vector<categoryEntry> *list, int length)
{
	// Implements bubble sort for broad index
	categoryEntry temp;
	int iteration;
	int index;

        for (iteration = 1; iteration < length; iteration++)
        {
                for (index = 0; index < length - iteration; index++)
                {
                        if (list->at(index).title.compare(list->at(index + 1).title) > 0)
                        {
                                temp = list->at(index);
                                list->at(index) = list->at(index + 1);
                                list->at(index + 1) = temp;
                        }
                }
        }
}

void bubbleSort(vector<hymnEntry> *hymnList, int length)
{
	// Implements bubble sort for hymn list
	hymnEntry temp;
	int iteration;
	int index;

	for (iteration = 1; iteration < length; iteration++)
	{
		for (index = 0; index < length - iteration; index++)
		{
			if (hymnList->at(index).title.compare(hymnList->at(index + 1).title) > 0)
			{
				temp = hymnList->at(index);
				hymnList->at(index) = hymnList->at(index + 1);
				hymnList->at(index + 1) = temp;
			}
		}
	}
}
