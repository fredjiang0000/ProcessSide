/*
For more information, please refer to:

Chun-Te Chu <ctchu@u.washington.edu>
*/

#include "config.h"
#include <cstdlib>

Config::Config()
{
	ParametersNum = 20;

	//========================== Default value for each parameter ==============================
	Directory = new char[256];
	OutputImage = 0;
	InitBB = 0;
	BB = new int[4];
	KernelNumber = 2;
	Fraction1 = 0.5f;
	Fraction2 = 0.5f;
	Fraction3 = 0.5f;
	Fraction4 = 0.5f;
	BinInOneChannel = 8;
	ColorModel = BGR;
	HistogramChannelNumber = 1;
	ImageChannelNumber = 3;
	ConstraintThreshold = 3;
	CostThreshold = 0.7f;
	MaxIteration = 5;
	ScaleUpdate = 0;
	ScaleSmooth = 0.85f;
	EnableSmoothVelocity = 1;
	ConstantVelocity = new int[2];		ConstantVelocity[0] = 0;	ConstantVelocity[1] = 0;
}

Config::~Config()
{
//	if(BB)	delete [] BB;
//	if(Directory)	delete [] Directory;
//	if(ConstantVelocity)	delete [] ConstantVelocity;
}

bool Config::loadConf()
{

	std::ifstream infile;
	infile.open("tracking_config.txt");				// open a file

	if(!infile.good())								// if there is no such file
	{
		printf("[Error]: ConfLoader::loadConf(): open config file\n");
		return false;
	}

	char line[256];									// temp register for char
	char *pch;
	int count = 0;

	while(!infile.eof())							// if not end of file
	{
			infile.getline(line, 256);				// get the whole line into line[256]
			if(line[0] == '#')						// if this line starts with #, it is just a comment line
			{
				continue;
			}

			pch = strtok(line, " =");				// extract the char except " " and "="
			while(pch != NULL)
			{

				// Directory
				if(strcmp(pch, "Directory") == 0)		// if pch equals to Directory
				{
					pch = strtok(NULL, " =");			//  move the pointer to point to the next string without " " and "="
					strcpy(Directory, pch);
					count = count + 1;
					break;
				}

				// OutputImage
				else if(strcmp(pch, "OutputImage") == 0)
				{
					pch = strtok(NULL, " =");
					// check isdigit
					for(unsigned int i = 0; i < strlen(pch); ++i)		// in this string, check if all of the characters are digits
					{
							if(isdigit(pch[i]) == 0)		// if one of them is not a digit, then there is an error
							{
								printf("[Error]: ConfLoader::loadConf(): OutputImage is not an integer\n");
								return false;
							}
					}
					OutputImage = atoi(pch);
					count = count + 1;
					break;
				}

				// InitBB
				else if(strcmp(pch, "InitBB") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): InitBB is not an integer\n");
								return false;
							}
					}
					InitBB = atoi(pch);
					count = count + 1;
					break;
				}

				// BB
				else if(strcmp(pch, "BB") == 0)
				{
					for(int BBi = 0; BBi < 4; ++BBi)
					{
						pch = strtok(NULL, " =");
						for(unsigned int i = 0; i < strlen(pch); i++)
						{
								if(isdigit(pch[i]) == 0)
								{
									printf("[Error]: ConfLoader::loadConf(): %d of BB is not an integer\n", i);
									return false;
								}
						}
						BB[BBi] = atoi(pch);
					}
					count = count + 1;
					break;
				}

				// KernelNumber
				else if(strcmp(pch, "KernelNumber") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): KernelNumber is not an integer\n", i);
								return false;
							}
					}
					KernelNumber = atoi(pch);
					count = count + 1;
					break;
				}
				// Fraction1
				else if(strcmp(pch, "Fraction1") == 0)
				{
					pch = strtok(NULL, " =");
					Fraction1 = atof(pch);
					count = count + 1;
					break;
				}
				// Fraction2
				else if(strcmp(pch, "Fraction2") == 0)
				{
					pch = strtok(NULL, " =");
					Fraction2 = atof(pch);
					count = count + 1;
					break;
				}
				// Fraction3
				else if(strcmp(pch, "Fraction3") == 0)
				{
					pch = strtok(NULL, " =");
					Fraction3 = atof(pch);
					count = count + 1;
					break;
				}
				// Fraction4
				else if(strcmp(pch, "Fraction4") == 0)
				{
					pch = strtok(NULL, " =");
					Fraction4 = atof(pch);
					count = count + 1;
					break;
				}
				// BinInOneChannel
				else if(strcmp(pch, "BinInOneChannel") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): BinInOneChannel is not an integer\n");
								return false;
							}
					}
					BinInOneChannel = atoi(pch);
					count = count + 1;
					break;
				}
				// ColorModel
				else if(strcmp(pch, "ColorModel") == 0)
				{
					pch = strtok(NULL, " =");
					std::string model(pch);
					if(model.compare("GREY") == 0)
					{
						ColorModel = GREY;
					}
					else if(model.compare("HSV") == 0)
					{
						ColorModel = HSV;
					}
					else if(model.compare("BGR") == 0)
					{
						ColorModel = BGR;
					}
					else
					{
						printf("[Error]: ConfLoader::loadConf(): ColorModel is not valid\n");
						return false;
					}
					count = count + 1;
					break;
				}
				// HistogramChannelNumber
				else if(strcmp(pch, "HistogramChannelNumber") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): HistogramChannelNumber is not an integer\n");
								return false;
							}
					}
					HistogramChannelNumber = atoi(pch);
					count = count + 1;
					break;
				}
				// ImageChannelNumber
				else if(strcmp(pch, "ImageChannelNumber") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): ImageChannelNumber is not an integer\n");
								return false;
							}
					}
					ImageChannelNumber = atoi(pch);
					count = count + 1;
					break;
				}
				// ConstraintThreshold
				else if(strcmp(pch, "ConstraintThreshold") == 0)
				{
					pch = strtok(NULL, " =");
					ConstraintThreshold = atof(pch);
					count = count + 1;
					break;
				}
				// CostThreshold
				else if(strcmp(pch, "CostThreshold") == 0)
				{
					pch = strtok(NULL, " =");
					CostThreshold = atof(pch);
					count = count + 1;
					break;
				}
				// MaxIteration
				else if(strcmp(pch, "MaxIteration") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): MaxIteration is not an integer\n");
								return false;
							}
					}
					MaxIteration = atoi(pch);
					count = count + 1;
					break;
				}
				// ScaleUpdate
				else if(strcmp(pch, "ScaleUpdate") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): ScaleUpdate is not an integer\n");
								return false;
							}
					}
					ScaleUpdate = atoi(pch);
					count = count + 1;
					break;
				}
				// ScaleSmooth
				else if(strcmp(pch, "ScaleSmooth") == 0)
				{
					pch = strtok(NULL, " =");
					ScaleSmooth = atof(pch);
					count = count + 1;
					break;
				}
				// EnableSmoothVelocity
				else if(strcmp(pch, "EnableSmoothVelocity") == 0)
				{
					pch = strtok(NULL, " =");
					for(unsigned int i = 0; i < strlen(pch); ++i)
					{
							if(isdigit(pch[i]) == 0)
							{
								printf("[Error]: ConfLoader::loadConf(): EnableSmoothVelocity is not an integer\n");
								return false;
							}
					}
					EnableSmoothVelocity = atoi(pch);
					count = count + 1;
					break;
				}
				// ConstantVelocity
				else if(strcmp(pch, "ConstantVelocity") == 0)
				{
					for(int ConstantVelocityi = 0; ConstantVelocityi < 2; ++ConstantVelocityi)
					{
						pch = strtok(NULL, " =");
						for(unsigned int i = 0; i < strlen(pch); ++i)
						{
								if(isdigit(pch[i]) == 0)
								{
									printf("[Error]: ConfLoader::loadConf(): %d of ConstantVelocity is not an integer\n", i);
									return false;
								}
						}
						ConstantVelocity[ConstantVelocityi] = atoi(pch);
					}
					count = count + 1;
					break;
				}
				else
				{
					printf("[Error]: ConfLoader::loadConf(): undefined parameter: %s\n", pch);
					return false;
				}
		}
	}

	infile.close();

	if(count != ParametersNum)
	{
		printf("[Error]: ConfLoader::loadConf(): parameters loading is not complete\n");
		return false;
	}

	return true;
}
