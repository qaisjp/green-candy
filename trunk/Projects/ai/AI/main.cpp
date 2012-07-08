#include "StdInc.h"

enum eSentenceType
{
	SENTENCE_ANSWER,
	SENTENCE_QUESTION,
	SENTENCE_COMMAND,	// Not necessarily a command, ~like a shout-out
	SENTENCE_NORMAL
};

class Sentence
{
public:
	Sentence()
	{
		m_type = SENTENCE_NORMAL;
	}

	Sentence( const std::string& content, eSentenceType type )
	{
		// We have to parse the content now
		const char *stream = content.c_str();
		std::string token;

		for (;;)
		{
			const char c = *stream++;

			// Any unreadable character should split the tokens!
			if ( c <= ' ' )
			{
				if ( token.empty() )
				{
					if ( c == '\0' )
						break;
					
					continue;
				}

				m_tokens.push_back( token );

				if ( c == '\0' )
					break;

				token.clear();
				continue;
			}

			token += c;
		}

		m_type = type;
	}

	~Sentence()
	{
	}

	std::vector <std::string> m_tokens;
	eSentenceType m_type;
};

// Function to filter sentences out of a stream
static inline bool parsesentence( const char*& seek, Sentence& out )
{
	std::string content;

	while ( true )
	{
		const char c = *seek++;

		switch( c )
		{
		// End of the stream
		case 0:
			seek--;

			// If there is no new sentence and the stream ended, we have to quit reading
			if ( content.empty() )
				return false; // ... indicated by false

			out = Sentence( content, SENTENCE_NORMAL );
			return true;
		// Some standard sentence.
		case '.':
			// We should not care about empty sentences
			if ( content.empty() )
				break;

			out = Sentence( content, SENTENCE_NORMAL );
			return true;
		// The guy wants to express something!
		case '!':
			if ( content.empty() )
				break;

			out = Sentence( content, SENTENCE_COMMAND );
			return true;
		// We are getting some question!
		case '?':
			if ( content.empty() )
				break;

			out = Sentence( content, SENTENCE_QUESTION );
			return true;
		}

		content += c;
	}

	return true;
}

int main( int argc, char *argv[] )
{
	std::vector <char> buffer;

	// Initialize the filesystem library
	new CFileSystem;

	// Read the letter file into a std vector
	if ( !fileRoot->ReadToBuffer( "letter.txt", buffer ) )
		return EXIT_FAILURE;

	// Zero terminate C string
	buffer.push_back( '\0' );

	// Plant the iterator, the sentence structure and an output variable :3
	const char *seek = &buffer[0];
	std::vector <Sentence> sentences;
	Sentence out;

	// Proceed through the buffer sentence by sentence
	while ( parsesentence( seek, out ) )
		sentences.push_back( out );

	// To halt the program :D
	getchar();
	return EXIT_SUCCESS;
}