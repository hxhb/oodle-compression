// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "OodleHandlerComponent.h"
#include "Commandlets/Commandlet.h"

#include "OodleTrainerCommandlet.generated.h"

// don't compile this in for non-editor
#define USE_OODLE_TRAINER_COMMANDLET (WITH_EDITOR)

/**
 * Commandlet for processing UE4 packet captures, through Oodle's training API, for generating compressed state dictionaries.
 *
 *
 * Primary Commands:
 *	- "AutoGenerateDictionaries Changelist":
 *		- Iterates every directory recursively within "*Game*\Saved\Oodle\Server", and uses all capture files within each directory,
 *			to generate a dictionary stored in "*Game*\Content\Oodle", named "*Game**DirectoryName*.udic".
 *
 *		- For example, packet captures in "OrionGame\Saved\Oodle\Server\Input", will be generated into a dictionary stored in
 *			"OrionGame\Content\Oodle\OrionGameInput.udic"
 *
 *		- Each folder within "*Game*\Content\Oodle", should contain at least 100mb of packet captures.
 *		
 *		- Changelist is an optional parameter than will only use upac files that contain the changelist in their filenames. If
 *			omitted, all files in the directory are used.
 *
 *
 * Secondary/Testing Commands:
 *	- "Enable":
 *		- Inserts the Oodle PacketHandler into the games packet handler component list, and initializes Oodle *Engine.ini settings
 *
 *
 *	- "MergePackets OutputFile PacketFile1,PacketFile2,PacketFileN":
 *		- Takes the specified packet capture files, and merges them into a single packet capture file
 *
 *	- "MergePackets OutputFile All Directory":
 *		- As above, but merges all capture files in the specified directory.
 *
 *
 *	- "GenerateDictionary OutputFile FilenameFilter Changelist PacketFile1,PacketFile2,PacketFileN":
 *		- Takes the specified packet capture files, with an optional filter for a substring of a filename and changelist filter
 *			(use "all" to ignore either of these filters), and uses them to generate a network compression dictionary
 *
 *	- "GenerateDictionary OutputFile FilenameFilter Changelist All Directory":
 *		- As above, but uses all capture files in the specified directory, to generate a network compression dictionary
 *
 *
 *	- "DebugDump OutputDirectory CaptureDirectory Changelist"
 *		- Recursively iterates all .ucap files in CaptureDirectory, and converts them to Oodle-example-code compatible .bin files,
 *			in OutputDirectory
 *
 *
 * @todo #JohnB: Unimplemented commands:
 *	- "PacketInfo PacketFile":
 *		- Outputs information about the packet file, such as the MB amount of data recorded, per net connection channel, and data types
 *		- @todo #JohnB: Only implement, if deciding to actually capture/track this kind of data
 */
UCLASS(config=Editor)
class UOodleTrainerCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	/** Whether or not compression testing should be performed after dictionary generation (uses up some of the packets) */
	UPROPERTY(config)
	bool bCompressionTest;

	/** Size of the hash table to use for the dictionary */
	UPROPERTY(config)
	int32 HashTableSize;

	/** Size of the dictionary to be generated */
	UPROPERTY(config)
	int32 DictionarySize;

	/** The number of random packet-selection trials to run, when generating the dictionary, to try and optimize the dictionary */
	UPROPERTY(config)
	int32 DictionaryTrials;

	/** The randomness, in percent, of random packet-selection trials */
	UPROPERTY(config)
	int32 TrialRandomness;

	/** The number of generations of random packet-selection trials  */
	UPROPERTY(config)
	int32 TrialGenerations;

	/** Whether or not random-trials have been disabled */
	UPROPERTY(config)
	bool bNoTrials;

public:
	UOodleTrainerCommandlet(const FObjectInitializer& ObjectInitializer);

	virtual int32 Main(const FString& Params) override;

#if USE_OODLE_TRAINER_COMMANDLET
	/**
	 * Handles the 'enable' command, which enables the Oodle packet handler component
	 *
	 * @return	Whether or not the command executed successfully
	 */
	static bool HandleEnable();

#if !UE_BUILD_SHIPPING || OODLE_DEV_SHIPPING
	/**
	 * Handles the 'MergePackets' command, which is used to merge multiple packet capture files
	 *
	 * @param OutputCapFile		The file which will contain the merged packets
	 * @param MergeList			The list of packet capture files to merge, or if bMergeAll, the directory containing these files
	 * @param bMergeAll			Whether or not MergeList refers to a directory, where all of the files it contains should be merged
	 * @return					Whether or not the command executed successfully
	 */
	bool HandleMergePackets(FString OutputCapFile, const TArray<FString>& MergeList);


	/**
	 * Handles the 'AutoGenerateDictionaries' command, which is used to automatically detect packet captures and produce dictionaries
	 *
	 * This is done, by iterating every directory within "*Game*\Saved\Oodle\Server", and using all capture files within each directory,
	 * to generate a dictionary name "*Game**DirectoryName*.udic", stored in "*Game*\Content\Oodle".
	 *
	 * For example, captures in "OrionGame\Saved\Oodle\Server\Input", generate "OrionGame\Content\Oodle\OrionGameInput.udic".
	 *
	 * @return	Whether or not the command executed successfully
	 */
	bool HandleAutoGenerateDictionaries(int32 ChangelistNumber);


	/**
	 * Handles the 'DebugDump' command, which is used to take a directory of .ucap files, and output a directory of Oodle-example-code
	 * compatible .bin files, in SourceDirectory
	 *
	 * NOTE: The directory structure of SourceDirectory is preserved
	 *
	 * @param OutputDirectory	The directory where the .bin files should be output to (directory structure is preserved)
	 * @param SourceDirectory	The directory where the .ucap files are located
	 * @param DumpList	The list of packet capture files to dump
	 */
	bool HandleDebugDumpPackets(FString OutputDirectory, FString SourceDirectory, const TArray<FString>& DumpList);



	/**
	 * Converts a list of capture files to merge, into a map of file archives vs file names (doing all necessary verification etc.)
	 * NOTE: OutMergeMap FArchive*'s must be deleted by the caller.
	 *
	 * @param MergeList			The list of capture files to merge (or the directory containing files to merge, if bMergeAll)
	 * @param OutMergeMap		The output map of file archives vs file names
	 * @param bMergeAll			If true, merges all capture files in the specified directory (with MergeList pointing to a directory)
	 * @param bAllowSingleFile	Whether or not the function will error when only a single file is found
	 * @return					Whether or not the list was successfully parsed, and merge map successfully assigned
	 */
	static bool GetMergeMapFromList(const TArray<FString>& FileList, TMap<FArchive*, FString>& OutMergeMap);

	/**
	 * Checks that the output file does not already exist, and prompts for an overwrite, if it does
	 *
	 * @param OutputFile	The output file string
	 * @return				Whether or not OutputFile is a valid path
	 */
	static bool VerifyOutputFile(FString OutputFile);

private:
	/**
	 * Recursively searches a directory for capture files with an optional filename filter and changelist and puts
	 * an array of the resulting files in OutFiles
	 *
	 * @param FilenameFilter Filters by this character sequence found in the filename. Using "" will find all files.
	 * @param ChangelistNumber Filters files with this changelist in their filename. Using -1 will find all files.
	 * @param StartDirectory The top-level directory to begin the search from
	 * @param OutFiles The list of capture files found
	 */
	void GetCaptureFiles(FString FilenameFilter, int32 ChangelistNumber, FString StartDirectory, TArray<FString>& OutFiles);

	/**
	 * Called by HandleAutoGenerateDictionaries, will generate a dictionary for the capture files in either in the "Input"
	 * or "Output" directory in Saved\Oodle
	 *
	 * @param bIsInput true if we are generating the dictionary for Input captures, false if it is Output
	 * @param ChangelistNumber OFilters files with this changelist in their filename. Using -1 will find all files.
	 */
	bool GenerateDictionary(bool bIsInput, int32 ChangelistNumber);

#endif // !UE_BUILD_SHIPPING || OODLE_DEV_SHIPPING
#endif // USE_OODLE_TRAINER_COMMANDLET
};


#if USE_OODLE_TRAINER_COMMANDLET && (!UE_BUILD_SHIPPING || OODLE_DEV_SHIPPING)
/**
 * FOodleDictionaryGenerator
 *
 * This class encapsulates dictionary generation, and splits it into multiple stages for readability
 */
class FOodleDictionaryGenerator
{
	/** Input/Parameter variables */
private:
	/** The path for outputting the generated dictionary */
	FString OutputDictionaryFile;

	/** Whether or not compression testing should be performed after dictionary generation (uses up some of the packets) */
	bool bCompressionTest;

	/** Size of the hash table to use for the dictionary */
	int32 HashTableSize;

	/** Size of the dictionary to be generated */
	int32 DictionarySize;

	/** The number of random packet-selection trials to run, when generating the dictionary, to try and optimize the dictionary */
	int32 DictionaryTrials;

	/** The randomness, in percent, of random packet-selection trials */
	int32 TrialRandomness;

	/** The number of generations of random packet-selection trials  */
	// @todo #JohnB: Examine this more closely, and document better (see email from Charles on 20/11/15)
	int32 TrialGenerations;

	/** Whether or not random-trials have been disabled */
	bool bNoTrials;


	/** Runtime variables (Opaque) */
private:
	TMap<FArchive*, FString> MergeMap;


	TArray<uint8*>	DictionaryPackets;
	TArray<int32>	DictionaryPacketSizes;
	uint32			DictionaryPacketBytes;
	TArray<uint8*>	DictionaryTestPackets;
	TArray<int32>	DictionaryTestPacketSizes;
	uint32			DictionaryTestPacketBytes;
	TArray<uint8*>	TrainerPackets;
	TArray<int32>	TrainerPacketSizes;
	uint32			TrainerPacketBytes;
	TArray<uint8*>	CompressionTestPackets;
	TArray<int32>	CompressionTestPacketSizes;
	uint32			CompressionTestPacketBytes;

	bool bDictionaryTestOverflow;

	/** Whether or not to do a debug-dump of the raw packet data, instead of generation a dictionary (also skips randomization) */
	bool bDebugDump;

public:
	/**
	 * Base constructor
	 */
	FOodleDictionaryGenerator()
		: OutputDictionaryFile()
		, bCompressionTest(false)
		, HashTableSize(0)
		, DictionarySize(0)
		, DictionaryTrials(0)
		, TrialRandomness(0)
		, TrialGenerations(0)
		, bNoTrials(false)
		, MergeMap()
		, DictionaryPacketBytes(0)
		, DictionaryTestPacketBytes(0)
		, TrainerPacketBytes(0)
		, CompressionTestPacketBytes(0)
		, bDictionaryTestOverflow(false)
		, bDebugDump(false)
	{
	}

	/**
	 * Primary function handling generation of the dictionary
	 *
	 * @param InOutputDictionaryFile	The path for the final dictionary file
	 * @param InputCaptureFiles			The capture file to process, or if bMergeAll, the directory where all captures files are located
	 * @return							Whether or not the dictionary generation was successful
	 */
	bool BeginGenerateDictionary(FString InOutputDictionaryFile, const TArray<FString>& InputCaptureFiles);


private:
	/**
	 * Initialize the dictionary generate parameters and state
	 *
	 * @return	Whether or not initialization was successful
	 */
	bool InitGenerator();

	/**
	 * Reads the specified capture files, and loads/sorts them in memory, in preparation for processing
	 *
	 * @param InputCaptureFiles		As per BeginGenerateDictionary
	 * @param bMergeAll				As per BeginGenerateDictionary
	 * @return						Whether or not the capture file packets were read successfully
	 */
	bool ReadPackets(const TArray<FString>& InputCaptureFiles);

	/**
	 * Processes loaded packet data through the Oodle dictionary generation API, and then writes/compresses the result to the final file
	 *
	 * @return	Whether or not final dictionary generation was successful
	 */
	bool GenerateAndWriteDictionary();

	/**
	 * When debug dumping is enabled, dumps the read packets, instead of generating a dictionary
	 *
	 * @return	Whether or not debug dumping was successful
	 */
	bool DebugDumpPackets();

	/**
	 * Cleans up any leftover allocated memory
	 */
	void Cleanup();
};
#endif
