class hymn
{
public:


private:
	// Hymn number
	int number;
	// Hymn title
	string title;
	// Hymn composer
	string composer;
	// Hymn lyricist
	string lyricist;
	// Hymn meter
	string meter;
	// Hymn tune name
	string tune = "Unknown";
	// Hymn time signature
	string time;

	// TODO: Presently category and scripture are not
	// loaded.
	// Hymn category
	string category;
	// Hymn scripture references
	string scripture;

	// Hymn's staff skip amount
	string staffSkip;
	// Hymn's scale amount
	float scale = 0.00;

	// Hymn's ABC data
	string abcData;
};
