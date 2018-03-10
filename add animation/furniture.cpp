#include "furniture.h"

#pragma warning(disable:4503)

using namespace std;

string ZeroEvent;

Furniture::Furniture(vecstr furnitureformat, string formatname, int furniturecount, string curfilepath, animationInfo& animationinfo)
{
	// import registerAnimation information
	addOn = animationinfo.addOn;
	linkedOption = animationinfo.linkedOption;
	optionPicked = animationinfo.optionPicked;
	optionPickedCount = animationinfo.optionPickedCount;
	addition = animationinfo.addition;
	groupAddition = animationinfo.groupAddition;
	mixOptRegis = animationinfo.mixOptRegis;
	mixOptRever = animationinfo.mixOptRever;
	eventID = animationinfo.eventID;
	variableID = animationinfo.variableID;
	hasDuration = animationinfo.hasDuration;
	duration = animationinfo.duration;
	mainAnimEvent = animationinfo.mainAnimEvent;
	filename = animationinfo.filename;
	AnimObject = animationinfo.AnimObject;

	format = formatname;
	furniturelines = furnitureformat;
	furnitureCount = furniturecount;
	filepath = curfilepath;
}

vecstr Furniture::GetFurnitureLine(int& nFunctionID, ImportContainer& import, id eventid, id variableid, int& stateID, int stateCountMultiplier)
{
	vecstr generatedlines;
	vecstr recorder;

	newImport = const_cast<ImportContainer*>(&import);
	nextFunctionID = const_cast<int*>(&nFunctionID);
	lastState = const_cast<int*>(&stateID);

	bool skip = false; // mainly used by NEW
	bool freeze = false; // mainly used by CONDITION to freeze following CONDITION
	bool open = false;
	bool norElement = false;
	bool multi = false;
	int openOrder = -2;
	int condition = 0;
	int fixedStateID = stateID;
	__int64 openRange = 0;
	__int64 counter = 0;
	string multiOption;
	size_t elementLine = -1;
	unordered_map<int, bool> IsConditionOpened;
	vector<unordered_map<string, bool>> groupOptionPicked;

	strID = to_string(*nextFunctionID);

	for (unsigned int i = 0; i < 4 - strID.length(); i++)
	{
		strID = "0" + strID;
	}

	groupOptionPicked.reserve(groupAnimInfo.size());

	for (unsigned int i = 0; i < groupAnimInfo.size(); ++i)
	{
		groupOptionPicked.push_back(groupAnimInfo[i]->optionPicked);
	}

	IsConditionOpened[0] = true;
	generatedlines.reserve(furniturelines.size() + 10 * memory);

	for (unsigned int i = 0; i < furniturelines.size(); i++)
	{
		bool uniqueskip = false;
		bool elementCatch = false;
		string line = furniturelines[i];

		if (line.find("<!-- CONDITION START ^", 0) != string::npos)
		{
			condition++;

			if (!freeze)
			{
				if (!IsConditionOpened[condition])
				{
					if (isPassed(condition, IsConditionOpened))
					{
						size_t optionPosition = line.find("<!-- CONDITION START ^") + 22;
						string conditionLine = line.substr(optionPosition, line.find("^ -->", optionPosition) - optionPosition);

						if (newCondition(conditionLine, groupOptionPicked, i + 1))
						{
							skip = false;
							IsConditionOpened[condition] = true;
						}
						else
						{
							skip = true;
						}
					}
				}
				else
				{
					skip = true;
					freeze = true;
				}
			}

			uniqueskip = true;
		}
		else if (line.find("<!-- CONDITION ^", 0) != string::npos)
		{
			if (condition == 0)
			{
				cout << "ERROR(1118): Opening of condition is required. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
				error = true;
				generatedlines.shrink_to_fit();
				return generatedlines;
			}

			if (!freeze)
			{
				if (!IsConditionOpened[condition])
				{
					if (isPassed(condition, IsConditionOpened))
					{
						size_t optionPosition = line.find("<!-- CONDITION ^") + 16;
						string option = line.substr(optionPosition, line.find("^ -->", optionPosition) - optionPosition);

						if (newCondition(option, groupOptionPicked, i + 1))
						{
							skip = false;
							IsConditionOpened[condition] = true;
						}
						else
						{
							skip = true;
						}
					}
				}
				else
				{
					skip = true;
					freeze = true;
				}
			}

			uniqueskip = true;
		}
		else if (line.find("<!-- CONDITION -->", 0) != string::npos)
		{
			if (condition == 0)
			{
				cout << "ERROR(1120): Opening of condition is required. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
				error = true;
				generatedlines.shrink_to_fit();
				return generatedlines;
			}

			if (!freeze)
			{
				if (!IsConditionOpened[condition])
				{
					if (isPassed(condition, IsConditionOpened))
					{
						skip = false;
						IsConditionOpened[condition] = true;
						uniqueskip = true;
						size_t conditionPosition = line.find("<!-- CONDITION") + 14;
						string replacement1 = line.substr(0, conditionPosition + 1);
						string replacement2 = line.substr(conditionPosition);
						generatedlines.push_back(replacement1 + "START" + replacement2);
					}
					else
					{
						skip = true;
					}
				}
				else
				{
					skip = true;
					freeze = true;
				}
			}

			uniqueskip = true;
		}
		else if (line.find("<!-- NEW ^", 0) != string::npos && line.find("^ -->", 0) != string::npos && IsConditionOpened[condition])
		{
			if (!open)
			{
				string curOption = getOption(furniturelines[i], false);
				bool isNot = false;

				if (curOption[0] == '!')
				{
					isNot = true;
					curOption = curOption.substr(1);
				}

				vecstr optionInfo = GetOptionInfo(curOption, format, i + 1, lastOrder, groupAnimInfo, false, true, false, order);

				if (error)
				{
					generatedlines.shrink_to_fit();
					return generatedlines;
				}

				if (optionInfo[2] != "AnimObject")
				{
					if (groupAnimInfo[stoi(optionInfo[1])]->optionPicked[optionInfo[2]])
					{
						if (isNot)
						{
							skip = true;

						}
						else
						{
							open = true;
						}
					}
					else
					{
						// clear group number
						string previous = optionInfo[2];
						string templine = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

						if (groupAnimInfo[stoi(optionInfo[1])]->optionPicked[templine])
						{
							if (isNot)
							{
								skip = true;
							}
							else
							{
								open = true;
							}
						}
						else
						{
							string ID = boost::regex_replace(string(previous), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

							// animobject bypass
							if (previous == "AnimObject/" + ID)
							{
								if (groupAnimInfo[stoi(optionInfo[1])]->optionPicked[previous])
								{
									if (isNot)
									{
										skip = true;
									}
									else
									{
										open = true;
									}
								}
								else
								{
									if (isNot)
									{
										open = true;
									}
									else
									{
										skip = true;
									}
								}
							}
							else
							{
								if (isNot)
								{
									open = true;
								}
								else
								{
									skip = true;
								}

								if (error)
								{
									generatedlines.shrink_to_fit();
									return generatedlines;
								}
							}
						}
					}
				}
				else
				{
					cout << "ERROR(1150): General AnimObject cannot be used in non-multi new. Get Specific AnimObject instead. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
					skip = true;
				}
			}
			else
			{
				cout << "ERROR(1117): Unresolved order section. Closing of order is required. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
				error = true;
				generatedlines.shrink_to_fit();
				return generatedlines;
			}

			uniqueskip = true;
		}
		else if (line.find("<!-- NEW ^", 0) != string::npos && line.find("^ +% -->", 0) != string::npos && IsConditionOpened[condition])
		{
			if (!open)
			{
				string curOption = getOption(line, true);
				bool isNot = false;

				if (curOption[0] == '!')
				{
					isNot = true;
					curOption = curOption.substr(1);
				}

				while (true)
				{
					vecstr optionInfo = GetOptionInfo(curOption, format, i + 1, groupAnimInfo.size() - 1, groupAnimInfo, true, true, false, order);
					string animID = boost::regex_replace(string(curOption), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

					if (error)
					{
						generatedlines.shrink_to_fit();
						return generatedlines;
					}

					if (optionInfo[2] != "AnimObject/" + animID)
					{
						if (optionInfo[1].length() == 0)
						{
							recorder.reserve(furniturelines.size() / 5);
							open = true;
							multi = true;
							openOrder = -1;
							multiOption = optionInfo[2];
						}
						else
						{
							if (!groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
							{
								// animobject bypass
								if (optionInfo[2] == "AnimObject")
								{
									if (isNot)
									{
										skip = true;
									}
									else
									{
										recorder.reserve(furniturelines.size() / 5);
										open = true;
										multi = true;
										multiOption = optionInfo[2];
										openOrder = stoi(optionInfo[1]);
									}
								}
								else
								{
									// Check if current condition accepts other options that are linked
									if (isNot)
									{
										recorder.reserve(furniturelines.size() / 5);
										open = true;
										multi = true;
										multiOption = optionInfo[2];

										if (optionInfo[1].length() == 0)
										{
											openOrder = -1;
										}
										else
										{
											openOrder = stoi(optionInfo[1]);
										}
									}
									else
									{
										skip = true;
									}
								}
							}
							else
							{
								if (isNot)
								{
									skip = true;
								}
								else
								{
									recorder.reserve(furniturelines.size() / 5);
									open = true;
									multi = true;
									multiOption = optionInfo[2];
									openOrder = stoi(optionInfo[1]);
								}
							}
						}
					}
					else
					{
						cout << "WARNING: Specific AnimObject cannot be used in multi new. Use \"AnimObject\" instead. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
						skip = true;
					}

					break;
				}
			}
			else
			{
				cout << "ERROR(1115): Unresolved order section. Closing of order is required. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
				error = true;
				generatedlines.shrink_to_fit();
				return generatedlines;
			}

			uniqueskip = true;
		}
		else if (line.find("<!-- NEW ORDER ", 0) != string::npos && line.find(" -->", 0) != string::npos && IsConditionOpened[condition])
		{
			if (!open)
			{
				size_t orderPosition = line.find("<!-- NEW ORDER ") + 15;
				string curOrder = line.substr(orderPosition, line.find(" -->", orderPosition) - orderPosition);
				bool isNot = false;

				if (curOrder[0] == '!')
				{
					isNot = true;
					curOrder = curOrder.substr(1);
				}

				bool number = false;
				bool word = false;
				bool unknown = false;

				for (unsigned int j = 0; j < curOrder.size(); j++)
				{
					if (isalpha(curOrder[j]))
					{
						word = true;
					}
					else if (isalnum(curOrder[j]))
					{
						number = true;
					}
					else
					{
						unknown = true;
					}
				}

				if (word && number)
				{
					cout << "ERROR(1110): Invalid order. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
					error = true;
					generatedlines.shrink_to_fit();
					return generatedlines;
				}
				else if (unknown)
				{
					cout << "ERROR(1111): Invalid order. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
					error = true;
					generatedlines.shrink_to_fit();
					return generatedlines;
				}
				else if (word)
				{
					if (boost::iequals(curOrder, "last"))
					{
						if (!isLastOrder)
						{
							if (!isNot)
							{
								skip = true;
							}
						}
						else if (isNot)
						{
							skip = true;
						}
					}
					else if (boost::iequals(curOrder, "first"))
					{
						if (order != 0)
						{
							if (!isNot)
							{
								skip = true;
							}
						}
						else if (isNot)
						{
							skip = true;
						}
					}
					else
					{
						cout << "ERROR(1112): Invalid order. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
						error = true;
						generatedlines.shrink_to_fit();
						return generatedlines;
					}
				}
				else if (number)
				{
					if (order != stoi(curOrder))
					{
						if (!isNot)
						{
							skip = true;
						}
					}
					else if (isNot)
					{
						skip = true;
					}
				}
				else
				{
					cout << "ERROR(1113): Invalid order. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
					error = true;
					generatedlines.shrink_to_fit();
					return generatedlines;
				}
			}
			else
			{
				cout << "ERROR(1114): Unresolved order section. Closing of order is required. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
				error = true;
				generatedlines.shrink_to_fit();
				return generatedlines;
			}

			uniqueskip = true;
		}
		else if (line.find("<!-- CLOSE -->", 0) != string::npos || line.find("<!-- CONDITION END -->", 0) != string::npos)
		{
			uniqueskip = true;
		}

		if (!skip && !uniqueskip && !freeze)
		{
			while (true)
			{
				if (multi)
				{
					recorder.push_back(line);
					break;
				}

				if (!hasDuration && isEnd && line.find("\t\t<hkparam name=\"relativeToEndOfClip\">false</hkparam>", 0) != string::npos)
				{
					line.replace(line.find("\t\t<hkparam name=\"relativeToEndOfClip\">false</hkparam>") + 38, 5, "true");
					isEnd = false;
				}

				// compute numelements
				if (line.find("<hkparam name=\"") != string::npos && line.find("numelements=\"") != string::npos && line.find("</hkparam>") == string::npos && line.find("<!-- COMPUTE -->", line.find("numelements=\"")) != string::npos)
				{
					if (!norElement)
					{
						norElement = true;
						elementCatch = true;
						string templine = line.substr(0, line.find("<hkparam name=\"", 0));
						openRange = count(templine.begin(), templine.end(), '\t');
					}
					else
					{
						cout << "ERROR(1136): Multiple computation function running concurrently detected. Only 1 computation function can be done at a time" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
						error = true;
						generatedlines.shrink_to_fit();
						return generatedlines;
					}
				}
				else if (line.find("</hkparam>") != string::npos && norElement)
				{
					string templine = line.substr(0, line.find("</hkparam>"));
					__int64 range = count(templine.begin(), templine.end(), '\t');

					if (openRange == range)
					{
						string oldElement;

						if (generatedlines[elementLine].find("numelements=\"$elements$\">", 0) == string::npos)
						{
							size_t position = generatedlines[elementLine].find("numelements=\"") + 13;
							oldElement = generatedlines[elementLine].substr(position, generatedlines[elementLine].find("\">", position) - position);
						}
						else
						{
							oldElement = "$elements$";
						}

						if (oldElement != to_string(counter))
						{
							generatedlines[elementLine].replace(generatedlines[elementLine].find(oldElement), oldElement.length(), to_string(counter));
						}

						norElement = false;
						counter = 0;
						elementLine = -1;
					}
				}

				if (norElement)
				{
					string templine = line;

					if (templine.find("<hkobject>") != string::npos)
					{
						templine = templine.substr(0, templine.find("<hkobject>"));
						__int64 range = count(templine.begin(), templine.end(), '\t');

						if (range == openRange + 1)
						{
							counter++;
						}
					}
					else if (templine.find("\t\t\t#") != string::npos)
					{
						templine = templine.substr(0, templine.find("#", 0));
						__int64 reference = count(templine.begin(), templine.end(), '\t');

						if (reference == openRange + 1)
						{
							__int64 number = count(line.begin(), line.end(), '#');
							counter += number;
						}
					}
				}

				if (line.find("<hkparam name=\"variableIndex\">$variableID[AV[variable]]$</hkparam>") != string::npos)
				{
					counter = counter;
				}

				if (line.find("$") != string::npos)
				{
					// set animation ID
					if (line.find("$%$", 0) != string::npos)
					{
						line.replace(line.find("$%$"), 3, to_string(furnitureCount));
					}

					// multi choice selection
					if (line.find("$MC$", 0) != string::npos)
					{
						multiChoice(line, groupOptionPicked, i + 1);

						if (error)
						{
							generatedlines.shrink_to_fit();
							return generatedlines;
						}
					}

					// set function ID
					if (line.find("MID$", 0) != string::npos)
					{
						int counter = sameWordCount(line, "MID$");

						for (int k = 0; k < counter; k++)
						{
							size_t MIDposition = line.find("MID$");
							string ID = boost::regex_replace(string(line.substr(MIDposition)), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
							string oldID = "MID$" + ID;


							if (line.find(oldID, MIDposition) != string::npos)
							{
								if (IDExist[oldID].length() > 0)
								{
									ID = IDExist[oldID];
								}
								else
								{
									IDExist[oldID] = strID;
									ID = strID;
									newID();
								}

								string templine = line;
								templine.replace(templine.find("MID$", MIDposition), 3, format);
								templine = templine.substr(templine.find(format + "$", MIDposition), format.length() + oldID.length() - 3);
								subFunctionIDs[templine] = ID;
								line.replace(line.find("MID$", MIDposition), oldID.length(), ID);
							}
							else
							{
								cout << "ERROR(1135): Invalid ID. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
								error = true;
								generatedlines.shrink_to_fit();
								return generatedlines;
							}
						}
					}

					processing(line, format, i + 1, eventid, variableid, fixedStateID, stateCountMultiplier);

					if (line.length() == 0)
					{
						error = true;
						generatedlines.shrink_to_fit();
						return generatedlines;
					}
				}

				generatedlines.push_back(line);

				if (elementCatch)
				{
					elementLine = generatedlines.size() - 1;
				}

				break;
			}
		}
				
		if (line.find("<!-- CLOSE -->", 0) != string::npos && IsConditionOpened[condition])
		{
			if (skip)
			{
				skip = false;
			}
			else
			{
				if (multi)
				{
					int size;

					if (openOrder == -2)
					{
						openOrder = 0;
						size = 1;
					}
					else if (openOrder == -1)
					{
						openOrder = 0;
						size = int(groupAnimInfo.size());
					}
					else
					{
						size = openOrder + 1;
					}

					for (int animMulti = openOrder; animMulti < size; ++animMulti)
					{
						for (int optionMulti = 0; optionMulti < groupAnimInfo[animMulti]->optionPickedCount[multiOption]; ++optionMulti)
						{
							for (unsigned int k = 0; k < recorder.size(); k++)
							{
								string newline = recorder[k];

								if (newline.find("$") != string::npos)
								{
									// set animation ID
									if (newline.find("$%$", 0) != string::npos)
									{
										newline.replace(newline.find("$%$"), 3, to_string(furnitureCount));
									}

									// multi choice selection
									if (newline.find("$MC$", 0) != string::npos)
									{
										multiChoice(newline, groupOptionPicked, i + 1);

										if (error)
										{
											generatedlines.shrink_to_fit();
											return generatedlines;
										}
									}

									// set function ID
									if (newline.find("MID$", 0) != string::npos)
									{
										int counter = sameWordCount(newline, "MID$");

										for (int k = 0; k < counter; k++)
										{
											size_t MIDposition = newline.find("MID$");
											string ID = boost::regex_replace(string(newline.substr(MIDposition)), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
											string oldID = "MID$" + ID;


											if (newline.find(oldID, MIDposition) != string::npos)
											{
												if (IDExist[oldID].length() > 0)
												{
													ID = IDExist[oldID];
												}
												else
												{
													IDExist[oldID] = strID;
													ID = strID;
													newID();
												}

												string templine = newline;
												templine.replace(templine.find("MID$", MIDposition), 3, format);
												templine = templine.substr(templine.find(format + "$", MIDposition), format.length() + oldID.length() - 3);
												subFunctionIDs[templine] = ID;
												newline.replace(newline.find("MID$", MIDposition), oldID.length(), ID);
											}
											else
											{
												cout << "ERROR(1135): Invalid ID. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
												error = true;
												generatedlines.shrink_to_fit();
												return generatedlines;
											}
										}
									}
								}
								if (newline.find("%", 0) != string::npos)
								{
									newline.replace(newline.find("%", 0), 1, to_string(optionMulti));
								}

								if (isEnd)
								{
									if (newline.find("\t\t<hkparam name=\"relativeToEndOfClip\">false</hkparam>", 0) != string::npos)
									{
										newline.replace(newline.find("\t\t<hkparam name=\"relativeToEndOfClip\">false</hkparam>") + 38, 5, "true");
										isEnd = false;
									}
								}
								
								// compute numelements
								if (norElement)
								{
									string templine = newline;

									if (templine.find("<hkobject>") != string::npos)
									{
										templine = templine.substr(0, templine.find("<hkobject>"));
										__int64 range = count(templine.begin(), templine.end(), '\t');

										if (range == openRange + 1)
										{
											counter++;
										}
									}
									else if (templine.find("\t\t\t#") != string::npos)
									{
										templine = templine.substr(0, templine.find("#", 0));
										__int64 reference = count(templine.begin(), templine.end(), '\t');

										if (reference == openRange + 1)
										{
											__int64 number = count(newline.begin(), newline.end(), '#');
											counter += number;
										}
									}
								}
								
								if (newline.find("$") != string::npos)
								{
									processing(newline, format, i + 1 - int(recorder.size()) + k, eventid, variableid, fixedStateID, stateCountMultiplier, optionMulti, animMulti, multiOption);

									if (error)
									{
										generatedlines.shrink_to_fit();
										return generatedlines;
									}
								}

								generatedlines.push_back(newline);
							}
						}
					}
					
					multiOption.clear();
					openOrder = -2;
				}

				recorder.clear();
			}

			multi = false;
			open = false;
		}
		else if (line.find("<!-- CONDITION END -->", 0) != string::npos)
		{
			if (condition == 0)
			{
				cout << "ERROR(1119): Unable to close condition. No opened condition is found. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << i + 1 << endl << endl;
				error = true;
				generatedlines.shrink_to_fit();
				return generatedlines;
			}

			if (freeze && IsConditionOpened[condition])
			{
				// generatedlines.push_back(line);
				freeze = false;
			}

			if (isPassed(condition, IsConditionOpened))
			{
				skip = false;
			}
			else
			{
				skip = true;
			}

			IsConditionOpened[condition] = false;
			condition--;
		}

		if (error)
		{
			generatedlines.shrink_to_fit();
			return generatedlines;
		}
	}

	IsConditionOpened[0] = false;

	for (auto it = IsConditionOpened.begin(); it != IsConditionOpened.end(); ++it)
	{
		if (it->second)
		{
			cout << "ERROR(1145): Unresolved option section. Closing of option is required. Please contact the template creator" << endl << "Template: " << format << endl << endl;
			error = true;
			generatedlines.shrink_to_fit();
			return generatedlines;
			break;
		}
	}

	if (condition != 0)
	{
		cout << "ERROR(1122): Unresolved condition. Closure of condition is not found. Please contact the template creator" << endl << "Template: " << format << endl << endl;
		error = true;
		generatedlines.shrink_to_fit();
		return generatedlines;
	}

	for (auto it = addOn.begin(); it != addOn.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); ++i)
		{
			if (addition[it->first][it->second[i]].length() != 0)
			{
				subFunctionIDs[format + "[" + to_string(order) + "][" + it->first + "][" + it->second[i] + "]"] = addition[it->first][it->second[i]];
			}
			else
			{
				string optionName;

				if (mixOptRegis[it->first].length() != 0)
				{
					optionName = mixOptRegis[it->first];

					for (int k = 0; k < optionPickedCount[optionName]; ++k)
					{
						for (unsigned int j = 0; j < groupAddition[optionName][it->second[i]].size(); ++j)
						{
							subFunctionIDs[format + "[" + to_string(order) + "][" + optionName + "][" + it->second[i] + "][" + to_string(j) + "]"] = groupAddition[optionName][it->second[i]][j];
						}
					}
				}
				else
				{
					optionName = it->first;

					for (unsigned int j = 0; j < groupAddition[optionName][it->second[i]].size(); ++j)
					{
						subFunctionIDs[format + "[" + to_string(order) + "][" + optionName + "][" + it->second[i] + "][" + to_string(j) + "]"] = groupAddition[optionName][it->second[i]][j];
					}
				}

			}
		}
	}

	if (AnimObject.size() != 0)
	{
		for (auto it = AnimObject.begin(); it != AnimObject.end(); ++it)
		{
			subFunctionIDs[format + "[" + to_string(order) + "][AnimObject" + to_string(it->first) + "]"] = it->second;
		}
	}

	subFunctionIDs[format + "[" + to_string(order) + "][StateID]"] = to_string(fixedStateID);
	subFunctionIDs[format + "[" + to_string(order) + "][main_anim_event]"] = mainAnimEvent;
	subFunctionIDs[format + "[" + to_string(order) + "][File]"] = filepath + filename;

	if (generatedlines.back().length() != 0)
	{
		generatedlines.push_back("");
	}

	generatedlines.shrink_to_fit();
	return generatedlines;
}

vecstr Furniture::GetEventID()
{
	return eventID;
}

vecstr Furniture::GetVariableID()
{
	return variableID;
}

SSMap Furniture::GetNewIDs()
{
	return subFunctionIDs;
}

SSMap Furniture::GetMixOpt()
{
	return mixOptRegis;
}

vector<shared_ptr<animationInfo>> Furniture::GetGroupAnimInfo()
{
	return groupAnimInfo;
}

ImportContainer Furniture::GetAddition()
{
	return addition;
}

unordered_map<string, unordered_map<string, vecstr>> Furniture::GetGroupAddition()
{
	return groupAddition;
}

inline void Furniture::newID()
{
	(*nextFunctionID)++;
	strID = to_string(*nextFunctionID);

	for (unsigned int i = 0; i < 4 - strID.length(); i++)
	{
		strID = "0" + strID;
	}
}

void Furniture::addGroupAnimInfo(vector<shared_ptr<animationInfo>> animInfo)
{
	if (animInfo.size() != lastOrder + 1)
	{
		cout << ">> ERROR(1147): BUG FOUND!! Report to Nemesis' author immediately <<" << endl << endl;
		error = true;
		return;
	}

	groupAnimInfo = animInfo;
}

void Furniture::storeAnimObject(vecstr animobjects, string listFilename, int lineCount)
{
	size_t position;

	for (unsigned int i = 0; i < animobjects.size(); i++)
	{
		position = animobjects[i].find("/");

		string ObjectName = animobjects[i].substr(0, animobjects[i].find("/", position));
		int temp = stoi(animobjects[i].substr(position + 1, 2));

		if (temp != 1 && temp != 2)
		{
			cout << "ERROR(1144): Invalid AnimObject" << endl << "File: " << listFilename << endl << "Line: " << lineCount << endl;
			error = true;
			return;
		}

		optionPicked["AnimObject/" + to_string(temp)] = true;
		AnimObject[temp] = ObjectName;
	}
}

void Furniture::setZeroEvent(string eventname)
{
	ZeroEvent = eventname;
}

void Furniture::setLastOrder(int curLastOrder)
{
	lastOrder = curLastOrder;

	if (order == curLastOrder)
	{
		isLastOrder = true;
	}
}

void Furniture::setOrder(int curOrder)
{
	order = curOrder;
}

int Furniture::getOrder()
{
	return order;
}

bool Furniture::isLast()
{
	return isLastOrder;
}

void Furniture::multiChoice(string& line, vector<unordered_map<string, bool>> groupOptionPicked, int numline)
{
	if (line.find("<!-- ", 0) != string::npos)
	{
		size_t nextposition = 0;
		int choicecount = 0;

		while (true)
		{
			if (line.find("<!--", nextposition) != string::npos)
			{
				choicecount++;
				nextposition = line.find("<!-- ", nextposition) + 1;
			}
			else
			{
				break;
			}
		}

		if (line.find("*", 0) != string::npos || line.find("@", 0) != string::npos || line.find("%", 0) != string::npos)
		{
			cout << "ERROR(1100): Invalid character input detected" << endl << "Template: " << format << endl << "Line: " << numline << endl << endl;
			error = true;
			return;
		}

		nextposition = 0;

		for (int i = 0; i < choicecount; i++)
		{
			vecstr opt;
			vector<char> storechar;
			nextposition = line.find("<!-- ", nextposition) + 5;
			string tempstr = line.substr(nextposition, line.find(" ", nextposition) - nextposition);

			if (i == choicecount - 1 && tempstr.find("$") != string::npos)
			{
				tempstr = line.substr(nextposition, line.find(" -->", nextposition) - nextposition);
			}

			if (line.find("<!-- " + tempstr + " -->", 0) == string::npos)
			{
				if (newCondition(tempstr, groupOptionPicked, numline))
				{
					nextposition = line.find(" ", nextposition) + 1;
					string output = line.substr(nextposition, line.find(" -->", nextposition) - nextposition);
					line.replace(line.find("$MC$", 0), 4, output);
					line = line.substr(0, line.find("</hkparam>") + 10);
					return;
				}
			}
			else if (i == choicecount - 1)
			{
				line.replace(line.find("$MC$", 0), 4, tempstr);
				line = line.substr(0, line.find("</hkparam>") + 10);
				return;
			}
			else
			{
				cout << "ERROR(1104): Invalid template condition" << endl << "Template: " << format << endl << "Line: " << numline << endl << endl;
				error = true;
				return;
			}
		}

		line.replace(line.find("$MC$", 0), 4, "null");
	}
	else
	{
		line.replace(line.find("$MC$", 0), 4, "null");
	}
}

bool Furniture::newCondition(string condition, vector<unordered_map<string, bool>> groupOptionPicked, int numline)
{
	if (condition[0] == '(')
	{
		size_t backB = condition.find(")", 0);
		string inHouse = condition.substr(1, backB - 1);
		size_t x = inHouse.find("&");
		size_t y = inHouse.find("|");

		bool inHouseResult;

		if (x == -1 && y == -1)
		{
			bool isNot = false;

			if (inHouse[0] == '!')
			{
				isNot = true;
				inHouse = inHouse.substr(1);
			}

			vecstr optionInfo = GetOptionInfo(inHouse, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

			if (error)
			{
				return false;
			}

			if (optionInfo[2][0] == '^')
			{
				string conditionOrder;

				while (true)
				{
					if (isalpha(optionInfo[2][1]))
					{
						conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

						if (conditionOrder == "last")
						{
							if (isNot)
							{
								inHouseResult = !isLastOrder;
							}
							else
							{
								inHouseResult = isLastOrder;
							}

							break;
						}
						else if (conditionOrder == "first")
						{
							conditionOrder = "0";
						}
					}
					else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
					{
						conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
					}
					else
					{
						cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << optionInfo[2] << endl << endl;
						error = true;
						return false;
					}

					if (order == stoi(conditionOrder))
					{
						inHouseResult = !isNot;
					}
					else
					{
						inHouseResult = isNot;
					}

					break;
				}
			}
			else
			{
				if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
				{
					inHouseResult = !isNot;
				}
				else
				{
					// Check if current condition accepts other options that are linked
					inHouseResult = isNot;
				}
			}
		}
		else if (x == -1 || (x > y && y != -1))
		{
			string firstCondition = inHouse.substr(0, inHouse.find("|"));
			string secondCondition = inHouse.substr(inHouse.find("|") + 1);
			bool isNot = false;

			if (firstCondition[0] == '!')
			{
				isNot = true;
				firstCondition = firstCondition.substr(1);
			}

			vecstr optionInfo = GetOptionInfo(firstCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

			if (error)
			{
				return false;
			}

			if (optionInfo[2][0] == '^')
			{
				string conditionOrder;

				while (true)
				{
					if (isalpha(optionInfo[2][1]))
					{
						conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

						if (conditionOrder == "last")
						{
							if (isNot)
							{
								inHouseResult = !isLastOrder;
							}
							else
							{
								inHouseResult = isLastOrder;
							}

							break;
						}
						else if (conditionOrder == "first")
						{
							conditionOrder = "0";
						}
					}
					else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
					{
						conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
					}
					else
					{
						cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << optionInfo[2] << endl << endl;
						error = true;
						return false;
					}

					if (order == stoi(conditionOrder))
					{
						inHouseResult = !isNot;
					}
					else
					{
						inHouseResult = isNot;
					}

					if (!inHouseResult && newCondition(secondCondition, groupOptionPicked, numline))
					{
						inHouseResult = true;
					}

					break;
				}
			}
			else
			{
				if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
				{
					inHouseResult = !isNot;
				}
				else
				{
					inHouseResult = isNot;
				}
				
				if (!inHouseResult && newCondition(secondCondition, groupOptionPicked, numline))
				{
					inHouseResult = true;
				}
			}
		}
		else if (y == -1 || (x < y && x != -1))
		{
			string firstCondition = inHouse.substr(0, inHouse.find("&"));
			string tempSecondCondition = inHouse.substr(inHouse.find("&") + 1);
			string secondCondition;
			bool isNot = false;

			if (firstCondition[0] == '!')
			{
				isNot = true;
				firstCondition = firstCondition.substr(1);
			}

			vecstr optionInfo = GetOptionInfo(firstCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

			if (error)
			{
				return false;
			}

			size_t x = tempSecondCondition.find("&");
			size_t y = tempSecondCondition.find("|");

			if ((x == -1 || x > y) && y != -1)
			{
				secondCondition = tempSecondCondition.substr(0, tempSecondCondition.find("|"));
				tempSecondCondition = tempSecondCondition.substr(tempSecondCondition.find("|") + 1);

				if (optionInfo[2][0] == '^')
				{
					string conditionOrder;

					while (true)
					{
						if (isalpha(optionInfo[2][1]))
						{
							conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

							if (conditionOrder == "last")
							{
								if (isNot)
								{
									inHouseResult = !isLastOrder;
								}
								else
								{
									inHouseResult = isLastOrder;
								}

								break;
							}
							else if (conditionOrder == "first")
							{
								conditionOrder = "0";
							}
						}
						else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
						{
							conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
						}
						else
						{
							cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << optionInfo[2] << endl << endl;
							error = true;
							return false;
						}

						if (order == stoi(conditionOrder))
						{
							if (isNot)
							{
								inHouseResult = false;
							}
							else
							{
								inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
							}
						}
						else
						{
							if (isNot)
							{
								inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
							}
							else
							{
								inHouseResult = false;
							}
						}

						if (!inHouseResult && newCondition(tempSecondCondition, groupOptionPicked, numline))
						{
							inHouseResult = true;
						}

						break;
					}
				}
				else
				{
					if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
					{
						if (isNot)
						{
							inHouseResult = false;
						}
						else
						{
							inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
						}
					}
					else
					{
						if (isNot)
						{
							inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
						}
						else
						{
							inHouseResult = false;
						}
					}

					if (!inHouseResult && newCondition(tempSecondCondition, groupOptionPicked, numline))
					{
						inHouseResult = true;
					}
				}
			}
			else
			{
				secondCondition = tempSecondCondition;

				if (optionInfo[2][0] == '^')
				{
					string conditionOrder;

					while (true)
					{
						if (isalpha(optionInfo[2][1]))
						{
							conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

							if (conditionOrder == "last")
							{
								if (isNot)
								{
									inHouseResult = !isLastOrder;
								}
								else
								{
									inHouseResult = isLastOrder;
								}

								break;
							}
							else if (conditionOrder == "first")
							{
								conditionOrder = "0";
							}
						}
						else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
						{
							conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
						}
						else
						{
							cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << optionInfo[2] << endl << endl;
							error = true;
							return false;
						}

						if (order == stoi(conditionOrder))
						{
							if (isNot)
							{
								inHouseResult = false;
							}
							else
							{
								inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
							}
						}
						else
						{
							if (isNot)
							{
								inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
							}
							else
							{
								inHouseResult = false;
							}
						}

						break;
					}
				}
				else
				{
					if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
					{
						if (isNot)
						{
							inHouseResult = false;
						}
						else
						{
							inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
						}
					}
					else
					{
						if (isNot)
						{
							inHouseResult = newCondition(secondCondition, groupOptionPicked, numline);
						}
						else
						{
							inHouseResult = false;
						}
					}
				}
			}
		}
		else
		{
			cout << "ERROR(1105): Invalid template condition" << endl << "Template: " << format << endl << "Line: " << numline << endl << endl;
			error = true;
			return false;
		}

		string outHouse = condition.substr(backB + 1);

		x = outHouse.find("&");
		y = outHouse.find("|");

		if (x == -1 && y == -1)
		{
			return inHouseResult;
		}
		else if (x == -1 || (x > y && y != -1))
		{
			string secondCondition = outHouse.substr(outHouse.find("|") + 1);

			if (inHouseResult || newCondition(secondCondition, groupOptionPicked, numline))
			{
				return true;
			}
		}
		else if (y == -1 || (x < y && x != -1))
		{
			string secondCondition = inHouse.substr(inHouse.find("&") + 1);

			if (inHouseResult && newCondition(secondCondition, groupOptionPicked, numline))
			{
				return true;
			}
		}
		else
		{
			cout << "ERROR(1106): Invalid template condition" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << condition << endl << endl;
			error = true;
			return false;
		}
	}
	else
	{
		size_t x = condition.find("&");
		size_t y = condition.find("|");

		if (x == -1 && y == -1)
		{
			string conditionOrder = condition;
			bool isNot = false;

			if (conditionOrder[0] == '!')
			{
				isNot = true;
				conditionOrder = conditionOrder.substr(1);
			}

			vecstr optionInfo = GetOptionInfo(conditionOrder, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

			if (error)
			{
				return false;
			}

			if (optionInfo[2][0] == '^')
			{
				if (isalpha(optionInfo[2][1]))
				{
					optionInfo[2] = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

					if (optionInfo[2] == "last")
					{
						if (isNot)
						{
							return !isLastOrder;
						}
						else
						{
							return isLastOrder;
						}
					}
					else if (optionInfo[2] == "first")
					{
						optionInfo[2] = "0";
					}
				}
				else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
				{
					optionInfo[2] = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
				}
				else
				{
					cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << condition << endl << endl;
					error = true;
					return false;
				}

				if (order == stoi(optionInfo[2]))
				{
					return !isNot;
				}
				else
				{
					return isNot;
				}
			}
			else
			{
				if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
				{
					return !isNot;
				}
				else
				{
					return isNot;
				}
			}
		}
		else if (x == -1 || (x > y && y != -1))
		{
			string firstCondition = condition.substr(0, condition.find("|"));
			string secondCondition = condition.substr(condition.find("|") + 1);
			bool isNot = false;

			if (firstCondition[0] == '!')
			{
				isNot = true;
				firstCondition = firstCondition.substr(1);
			}

			vecstr optionInfo = GetOptionInfo(firstCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

			if (error)
			{
				return false;
			}

			if (optionInfo[2][0] == '^')
			{
				string conditionOrder;

				if (isalpha(optionInfo[2][1]))
				{
					conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

					if (conditionOrder == "last")
					{
						if (isLastOrder)
						{
							if (isNot)
							{
								conditionOrder = "-1";
							}
							else
							{
								return true;
							}
						}
						else
						{
							if (isNot)
							{
								return true;
							}
							else
							{
								conditionOrder = "-1";
							}
						}
					}
					else if (conditionOrder == "first")
					{
						conditionOrder = "0";
					}
				}
				else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
				{
					conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
				}
				else
				{
					cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
					error = true;
					return false;
				}

				if (order == stoi(conditionOrder))
				{
					if (!isNot)
					{
						return true;
					}
				}
				else
				{
					if (isNot)
					{
						return true;
					}
				}

				if (newCondition(secondCondition, groupOptionPicked, numline))
				{
					return true;
				}
			}
			else
			{
				if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
				{
					if (!isNot)
					{
						return true;
					}
				}
				else
				{
					if (isNot)
					{
						return true;
					}
				}

				if (newCondition(secondCondition, groupOptionPicked, numline))
				{
					return true;
				}
			}
		}
		else if (y == -1 || (x < y && x != -1))
		{
			string firstCondition = condition.substr(0, condition.find("&"));
			string secondCondition = condition.substr(condition.find("&") + 1);
			bool isNot = false;

			if (firstCondition[0] == '!')
			{
				isNot = true;
				firstCondition = firstCondition.substr(1);
			}

			vecstr optionInfo = GetOptionInfo(firstCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

			if (error)
			{
				return false;
			}

			if (y != -1)
			{
				if (secondCondition[0] == '(')
				{
					int position;
					int openCounter = 0;

					for (unsigned int i = 0; i < secondCondition.size(); i++)
					{
						if (secondCondition[i] == '(')
						{
							openCounter++;
						}
						else if (secondCondition[i] == ')')
						{
							openCounter--;

							if (openCounter == 0)
							{
								position = i + 1;
								break;
							}
						}
					}

					string thirdCondition = secondCondition.substr(position);
					secondCondition = secondCondition.substr(1, position - 2);
					
					if (thirdCondition.length() == 0)
					{
						if (optionInfo[2][0] == '^')
						{
							string conditionOrder;

							if (isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

								if (conditionOrder == "last")
								{
									if (isLastOrder)
									{
										if (isNot)
										{
											conditionOrder = "-1";
										}
										else
										{
											return true;
										}
									}
									else
									{
										if (isNot)
										{
											return true;
										}
										else
										{
											conditionOrder = "-1";
										}
									}
								}
								else if (conditionOrder == "first")
								{
									conditionOrder = "0";
								}
							}
							else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
							}
							else
							{
								cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
								error = true;
								return false;
							}

							if (order == stoi(conditionOrder))
							{
								if (!isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
							else
							{
								if (isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
						}
						else
						{
							if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
							{
								if (!isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
							else
							{
								if (isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
						}
					}
					else
					{
						char logic = thirdCondition[0];
						thirdCondition = thirdCondition.substr(1);

						if (logic == '&')
						{
							if (optionInfo[2][0] == '^')
							{
								string conditionOrder;

								if (isalpha(optionInfo[2][1]))
								{
									conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

									if (conditionOrder == "last")
									{
										if (isLastOrder)
										{
											if (isNot)
											{
												conditionOrder = "-1";
											}
											else
											{
												return true;
											}
										}
										else
										{
											if (isNot)
											{
												return true;
											}
											else
											{
												conditionOrder = "-1";
											}
										}
									}
									else if (conditionOrder == "first")
									{
										conditionOrder = "0";
									}
								}
								else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
								{
									conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
								}
								else
								{
									cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
									error = true;
									return false;
								}

								if (order == stoi(conditionOrder))
								{
									if (!isNot && newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
								else
								{
									if (isNot && newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
							else
							{
								if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
								{
									if (!isNot && newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
								else
								{
									if (isNot)
									{
										if (newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
										{
											return true;
										}
									}
								}
							}
						}
						else if (logic == '|')
						{
							if (optionInfo[2][0] == '^')
							{
								string conditionOrder;

								if (isalpha(optionInfo[2][1]))
								{
									conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

									if (conditionOrder == "last")
									{
										if (isLastOrder)
										{
											if (isNot)
											{
												conditionOrder = "-1";
											}
											else
											{
												return true;
											}
										}
										else
										{
											if (isNot)
											{
												return true;
											}
											else
											{
												conditionOrder = "-1";
											}
										}
									}
									else if (conditionOrder == "first")
									{
										conditionOrder = "0";
									}
								}
								else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
								{
									conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
								}
								else
								{
									cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
									error = true;
									return false;
								}

								if ((order == stoi(conditionOrder)))
								{
									if (!isNot && newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
								else
								{
									if (isNot && newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}

								if (newCondition(thirdCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}
							else
							{
								if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
								{
									if (!isNot && newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
								else
								{
									if (isNot)
									{
										if (newCondition(secondCondition, groupOptionPicked, numline))
										{
											return true;
										}
									}
								}

								if (newCondition(thirdCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}
						}
						else
						{
							cout << "ERROR(1101): Invalid template condition" << endl << "Template: " << format << endl << "Line: " << numline << endl << endl;
							error = true;
							return false;
						}
					}
				}
				else
				{
					x = secondCondition.find("&");
					y = secondCondition.find("|");

					if (x == -1 && y == -1)
					{
						if (optionInfo[2][0] == '^')
						{
							string conditionOrder;

							if (isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

								if (conditionOrder == "last")
								{
									if (isLastOrder)
									{
										if (isNot)
										{
											conditionOrder = "-1";
										}
										else
										{
											conditionOrder = to_string(order);
										}
									}
									else
									{
										if (isNot)
										{
											conditionOrder = to_string(order);
										}
										else
										{
											conditionOrder = "-1";
										}
									}
								}
								else if (conditionOrder == "first")
								{
									conditionOrder = "0";
								}
							}
							else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
							}
							else
							{
								cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
								error = true;
								return false;
							}

							if (order == stoi(conditionOrder)) 
							{
								if (!isNot)
								{
									bool isAnotherNot = false;

									if (secondCondition[0] == '!')
									{
										isAnotherNot = true;
										secondCondition = secondCondition.substr(1);
									}

									vecstr optionInfo2 = GetOptionInfo(secondCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

									if (error)
									{
										return false;
									}

									if (optionInfo2[2][0] == '^')
									{
										string anotherConditionOrder;

										if (isalpha(optionInfo2[2][1]))
										{
											anotherConditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

											if (anotherConditionOrder == "last")
											{
												if (isLastOrder)
												{
													if (isAnotherNot)
													{
														anotherConditionOrder = "-1";
													}
													else
													{
														return true;
													}
												}
												else
												{
													if (isAnotherNot)
													{
														return true;
													}
													else
													{
														anotherConditionOrder = "-1";
													}
												}
											}
											else if (anotherConditionOrder == "first")
											{
												anotherConditionOrder = "0";
											}
										}
										else if (isalnum(optionInfo2[2][1]) && !isalpha(optionInfo2[2][1]))
										{
											anotherConditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
										}
										else
										{
											cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << secondCondition << endl << endl;
											error = true;
											return false;

										}

										if (order == stoi(anotherConditionOrder))
										{
											if (!isAnotherNot)
											{
												return true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												return true;
											}
										}
									}
									else
									{
										if (groupOptionPicked[stoi(optionInfo2[1])][optionInfo2[2]])
										{
											if (!isAnotherNot)
											{
												return true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												return true;
											}
										}
									}
								}
							}
							else
							{
								if (isNot)
								{
									bool isAnotherNot = false;

									if (secondCondition[0] == '!')
									{
										isAnotherNot = true;
										secondCondition = secondCondition.substr(1);
									}

									vecstr optionInfo2 = GetOptionInfo(secondCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

									if (error)
									{
										return false;
									}

									if (optionInfo2[2][0] == '^')
									{
										string anotherConditionOrder;

										if (isalpha(optionInfo2[2][1]))
										{
											anotherConditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

											if (anotherConditionOrder == "last")
											{
												if (isLastOrder)
												{
													if (isAnotherNot)
													{
														anotherConditionOrder = "-1";
													}
													else
													{
														return true;
													}
												}
												else
												{
													if (isAnotherNot)
													{
														return true;
													}
													else
													{
														anotherConditionOrder = "-1";
													}
												}
											}
											else if (anotherConditionOrder == "first")
											{
												anotherConditionOrder = "0";
											}
										}
										else if (isalnum(optionInfo2[2][1]) && !isalpha(optionInfo2[2][1]))
										{
											anotherConditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
										}
										else
										{
											cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << secondCondition << endl << endl;
											error = true;
											return false;

										}

										if (order == stoi(anotherConditionOrder))
										{
											if (!isAnotherNot)
											{
												return true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												return true;
											}
										}
									}
									else
									{
										if (groupOptionPicked[stoi(optionInfo2[1])][optionInfo2[2]])
										{
											if (!isAnotherNot)
											{
												return true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												return true;
											}
										}
									}
								}
							}
						}
						else
						{
							if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
							{
								if (!isNot)
								{
									bool isAnotherNot = false;

									if (secondCondition[0] == '!')
									{
										isAnotherNot = true;
										secondCondition = secondCondition.substr(1);
									}

									vecstr optionInfo2 = GetOptionInfo(secondCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

									if (error)
									{
										return false;
									}

									if (optionInfo2[2][0] == '^')
									{
										string conditionOrder;

										if (isalpha(optionInfo2[2][1]))
										{
											conditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

											if (conditionOrder == "last")
											{
												if (isLastOrder)
												{
													if (isAnotherNot)
													{
														conditionOrder = "-1";
													}
													else
													{
														return true;
													}
												}
												else
												{
													if (isAnotherNot)
													{
														return true;
													}
													else
													{
														conditionOrder = "-1";
													}
												}
											}
											else if (conditionOrder == "first")
											{
												conditionOrder = "0";
											}
										}
										else if (isalnum(optionInfo2[2][1]) && !isalpha(optionInfo2[2][1]))
										{
											conditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
										}
										else
										{
											cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << secondCondition << endl << endl;
											error = true;
											return false;
										}

										if (order == stoi(conditionOrder))
										{
											if (!isAnotherNot)
											{
												true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												true;
											}
										}
									}
									else
									{
										if (groupOptionPicked[stoi(optionInfo2[1])][optionInfo2[2]])
										{
											if (!isAnotherNot)
											{
												return true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												return true;
											}
										}
									}
								}
							}
							else
							{
								if (isNot)
								{
									bool isAnotherNot = false;

									if (secondCondition[0] == '!')
									{
										isAnotherNot = true;
										secondCondition = secondCondition.substr(1);
									}

									vecstr optionInfo2 = GetOptionInfo(secondCondition, format, numline, groupOptionPicked.size() - 1, groupAnimInfo, false, true, false, order);

									if (error)
									{
										return false;
									}

									if (optionInfo2[2][0] == '^')
									{
										string conditionOrder;

										if (isalpha(optionInfo2[2][1]))
										{
											conditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

											if (conditionOrder == "last")
											{
												if (isLastOrder)
												{
													if (isAnotherNot)
													{
														conditionOrder = "-1";
													}
													else
													{
														return true;
													}
												}
												else
												{
													if (isAnotherNot)
													{
														return true;
													}
													else
													{
														conditionOrder = "-1";
													}
												}
											}
											else if (conditionOrder == "first")
											{
												conditionOrder = "0";
											}
										}
										else if (isalnum(optionInfo2[2][1]) && !isalpha(optionInfo2[2][1]))
										{
											conditionOrder = boost::regex_replace(string(optionInfo2[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
										}
										else
										{
											cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << secondCondition << endl << endl;
											error = true;
											return false;
										}

										if (order == stoi(conditionOrder))
										{
											if (!isAnotherNot)
											{
												true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												true;
											}
										}
									}
									else
									{
										if (groupOptionPicked[stoi(optionInfo2[1])][optionInfo2[2]])
										{
											if (!isAnotherNot)
											{
												return true;
											}
										}
										else
										{
											if (isAnotherNot)
											{
												return true;
											}
										}
									}
								}
							}
						}
					}
					else if (x == -1 || (x > y && y != -1))
					{
						size_t position = secondCondition.find("|") + 1;
						string thirdCondition = secondCondition.substr(position);
						secondCondition = secondCondition.substr(0, position - 1);

						if (optionInfo[2][0] == '^')
						{
							string conditionOrder;

							if (isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

								if (conditionOrder == "last")
								{
									if (isLastOrder)
									{
										if (isNot)
										{
											conditionOrder = "-1";
										}
										else
										{
											return true;
										}
									}
									else
									{
										if (isNot)
										{
											return true;
										}
										else
										{
											conditionOrder = "-1";
										}
									}
								}
								else if (conditionOrder == "first")
								{
									conditionOrder = "0";
								}
							}
							else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
							}
							else
							{
								cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
								error = true;
								return false;
							}

							if (order == stoi(conditionOrder))
							{
								if (!isNot && newCondition(secondCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}
							else
							{
								if (isNot && newCondition(secondCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}

							if (newCondition(thirdCondition, groupOptionPicked, numline))
							{
								return true;
							}
						}
						else
						{
							if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
							{
								if (!isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
							else
							{
								if (isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}

							if (newCondition(thirdCondition, groupOptionPicked, numline))
							{
								return true;
							}
						}
					}
					else if (y == -1 || (x < y && x != -1))
					{
						size_t position = secondCondition.find("&") + 1;
						string thirdCondition = secondCondition.substr(position);
						secondCondition = secondCondition.substr(0, position - 1);

						if (optionInfo[2][0] == '^')
						{
							string conditionOrder;

							if (isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

								if (conditionOrder == "last")
								{
									if (isLastOrder)
									{
										if (isNot)
										{
											conditionOrder = "-1";
										}
										else
										{
											return true;
										}
									}
									else
									{
										if (isNot)
										{
											return true;
										}
										else
										{
											conditionOrder = "-1";
										}
									}
								}
								else if (conditionOrder == "first")
								{
									conditionOrder = "0";
								}
							}
							else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
							{
								conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
							}
							else
							{
								cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
								error = true;
								return false;
							}

							if (order == stoi(conditionOrder))
							{
								if (!isNot && newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}
							else
							{
								if (isNot && newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}
						}
						else
						{
							if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
							{
								if (!isNot && newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
								{
									return true;
								}
							}
							else
							{
								if (isNot)
								{
									if (newCondition(secondCondition, groupOptionPicked, numline) && newCondition(thirdCondition, groupOptionPicked, numline))
									{
										return true;
									}
								}
							}
						}
					}
					else
					{
						cout << "ERROR(1103): Invalid template condition" << endl << "Template: " << format << endl << "Line: " << numline << endl << endl;
						error = true;
						return false;
					}
				}
			}
			else
			{
				if (optionInfo[2][0] == '^')
				{
					string conditionOrder;

					if (isalpha(optionInfo[2][1]))
					{
						conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^A-Za-z\\s]*([A-Za-z\\s]+).*"), string("\\1"));

						if (conditionOrder == "last")
						{
							if (isLastOrder)
							{
								if (isNot)
								{
									conditionOrder = "-1";
								}
								else
								{
									return true;
								}
							}
							else
							{
								if (isNot)
								{
									return true;
								}
								else
								{
									conditionOrder = "-1";
								}
							}
						}
						else if (conditionOrder == "first")
						{
							conditionOrder = "0";
						}
					}
					else if (isalnum(optionInfo[2][1]) && !isalpha(optionInfo[2][1]))
					{
						conditionOrder = boost::regex_replace(string(optionInfo[2]), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
					}
					else
					{
						cout << "ERROR(1138): Invalid condition. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << "Condition: " << firstCondition << endl << endl;
					}

					if (order == stoi(conditionOrder))
					{
						if (!isNot && newCondition(secondCondition, groupOptionPicked, numline))
						{
							return true;
						}
					}
					else
					{
						if (isNot && newCondition(secondCondition, groupOptionPicked, numline))
						{
							return true;
						}
					}
				}
				else
				{
					if (groupOptionPicked[stoi(optionInfo[1])][optionInfo[2]])
					{
						if (!isNot && newCondition(secondCondition, groupOptionPicked, numline))
						{
							return true;
						}
					}
					else
					{
						if (isNot)
						{
							if (newCondition(secondCondition, groupOptionPicked, numline))
							{
								return true;
							}
						}
					}
				}
			}
		}
		else
		{
			cout << "ERROR(1102): Invalid template condition" << endl << "Template: " << format << endl << "Line: " << numline << endl << endl;
			error = true;
			return false;
		}
	}

	return false;
}

void Furniture::processing(string& line, string masterFormat, int linecount, id eventid, id variableid, int fixedStateID, int stateCountMultiplier, int optionMulti, int animMulti, string multiOption)
{
	__int64 counter = count(line.begin(), line.end(), '$') / 2;
	size_t curPos = 0;
	bool multiAnim;

	if (animMulti != -1)
	{
		multiAnim = true;
	}
	else
	{
		multiAnim = false;
	}

	for (int i = 0; i < counter; ++i)
	{
		bool isChange = false;
		curPos = line.find("$", curPos + 1);
		string change = line.substr(curPos, line.find("$", curPos + 1) - curPos + 1);
		string oldChange = change;
		change = change.substr(1, change.length() - 2);
		curPos = line.find("$", curPos + 1);

		while (true)
		{
			// order equation
			if (change.find("(") != string::npos && change.find("L", change.find("(")) != string::npos && change.find(")", change.find("(")) != string::npos)
			{
				__int64 maths = count(change.begin(), change.end(), '(');

				if (maths != 0 && maths == count(change.begin(), change.end(), ')'))
				{
					size_t nextpos = change.find("(");

					for (__int64 j = 0; j < maths; ++j)
					{
						string equation = change.substr(nextpos, change.find(")", nextpos) - 1);
						string number = boost::regex_replace(string(equation), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

						if(equation != "(S+" + number + ")")
						{
							size_t equationLength = equation.length();

							if (equation.find("L") != string::npos)
							{
								__int64 maths2 = count(change.begin(), change.end(), 'L');

								for (__int64 k = 0; k < maths2; ++k)
								{
									equation.replace(equation.find("L"), 1, to_string(lastOrder));
								}
							}

							if (equation.find("N") != string::npos)
							{
								__int64 maths2 = count(change.begin(), change.end(), 'N');

								for (__int64 k = 0; k < maths2; ++k)
								{
									equation.replace(equation.find("N"), 1, to_string(order + 1));
								}
							}

							calculate(equation, error);

							if (stoi(equation) > groupAnimInfo.size() - 1 || stoi(equation) < 0)
							{
								cout << "ERROR(1148): \"Minimum\" in option_list.txt must be used and contain larger value than the 1st element being used. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "Option: " << change << endl << endl;
								error = true;
								return;
							}

							change.replace(nextpos, equationLength, equation);
						}

						nextpos = change.find("(", nextpos + 1);
					}
				}
			}

			// set animation end duration
			if (change.find("END", 0) != string::npos)
			{
				if (isEnd)
				{
					cout << "ERROR(1107): Invalid structure. \"relativeToEndOfClip\" is not found prior to another \"END\" command. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "Option: " << change << endl << endl;
					error = true;
					return;
				}

				if (change.find(format + "[][END]", 0) != string::npos)
				{
					if (animMulti == -1)
					{
						cout << "ERROR(1058): Invalid element. Only \"F\", \"N\", \"L\" is acceptable for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "Option: " << change << endl << endl;
						error = true;
						return;
					}

					if (groupAnimInfo[animMulti]->hasDuration)
					{
						change.replace(change.find(format + "[][END]"), 7 + format.length(), to_string(groupAnimInfo[animMulti]->duration));
					}
					else
					{
						change.replace(change.find(format + "[][END]"), 7 + format.length(), "0.000000");
						isEnd = true;
					}

					isChange = true;
				}

				if (change.find(format + "[F][END]", 0) != string::npos)
				{
					if (groupAnimInfo[0]->hasDuration)
					{
						change.replace(change.find(format + "[F][END]"), 8 + format.length(), to_string(groupAnimInfo[0]->duration));
					}
					else
					{
						change.replace(change.find(format + "[F][END]"), 8 + format.length(), "0.000000");
						isEnd = true;
					}

					isChange = true;
				}

				if (change.find(format + "[L][END]", 0) != string::npos)
				{
					if (groupAnimInfo[0]->hasDuration)
					{
						change.replace(change.find(format + "[L][END]"), 8 + format.length(), to_string(groupAnimInfo[lastOrder]->duration));
					}
					else
					{
						change.replace(change.find(format + "[L][END]"), 8 + format.length(), "0.000000");
						isEnd = true;
					}

					isChange = true;
				}

				if (change.find(format + "[") != string::npos)
				{
					string number = boost::regex_replace(string(change.substr(change.find(format + "[") + 1 + format.length())), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

					if (change.find(format + "[" + number + "][END]", 0) != string::npos)
					{
						if (stoi(number) > groupAnimInfo.size() - 1 || stoi(number) < 0)
						{
							cout << "ERROR(1155): Invalid order number. Enter number from 0 to " << lastOrder << ". Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "Option: " << change << endl << endl;
							error = true;
							return;
						}

						if (groupAnimInfo[stoi(number)]->hasDuration)
						{
							change.replace(change.find(format + "[" + number + "][END]"), 7 + number.length() + format.length(), to_string(groupAnimInfo[stoi(number)]->duration));
						}
						else
						{
							change.replace(change.find(format + "[" + number + "][END]"), 7, "0.000000");
							isEnd = true;
						}

						isChange = true;
					}
				}

				if (change.find("END", 0) != string::npos)
				{
					if (hasDuration)
					{
						change.replace(change.find("END"), 3, to_string(duration));
					}
					else
					{
						change.replace(change.find("END"), 3, "0.000000");
						isEnd = true;
					}

					isChange = true;
				}
			}
			
			// set state ID
			if (change.find("(S+", 0) != string::npos)
			{
				if (change.find(format + "[][(S+", 0) != string::npos)
				{
					string number = boost::regex_replace(string(change), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

					if (change.find(format + "[][(S+" + number + ")]", 0) != string::npos)
					{
						stateReplacer(change, fixedStateID + ((animMulti - order) * stateCountMultiplier), linecount, "", true);
						isChange = true;
					}

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[F][(S+", 0) != string::npos)
				{
					stateReplacer(change, fixedStateID - (order * stateCountMultiplier), linecount, "F", true);
					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[N][(S+", 0) != string::npos)
				{
					stateReplacer(change, fixedStateID + stateCountMultiplier, linecount, "N", true);
					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[L][(S+", 0) != string::npos)
				{
					stateReplacer(change, fixedStateID + ((lastOrder - order) * stateCountMultiplier), linecount, "L", true);
					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[") != string::npos)
				{
					string number = boost::regex_replace(string(change.substr(change.find(format + "[") + 1 + format.length())), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

					if (change.find(format + "[" + number + "][(S+") != string::npos)
					{
						string number2 = boost::regex_replace(string(change.substr(change.find(format + "[" + number + "][(S+"))), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
						{
							if (change.find(format + "[" + number + "][(S+" + number2 + ")]") != string::npos)
							{
								stateReplacer(change, fixedStateID + ((stoi(number) - order) * stateCountMultiplier), linecount, number, true);
							}
						}
					}
				}

				if (change.find("(S+", 0) != string::npos)
				{
					string number = boost::regex_replace(string(change), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

					if (change.find("(S+" + number + ")", 0) != string::npos)
					{
						stateReplacer(change, fixedStateID, linecount);
						isChange = true;
					}

					if (error)
					{
						return;
					}
				}
			}

			// set file path
			if (change.find("File", 0) != string::npos)
			{
				if (change.find(format + "[][File]", 0) != string::npos)
				{
					if (animMulti != -1)
					{
						change.replace(change.find(format + "[][File]"), 8 + format.length(), filepath + groupAnimInfo[animMulti]->filename);
						isChange = true;
					}
					else
					{
						cout << "ERROR(1146): Invalid reference. Specifying reference is required" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
						error = true;
						return;
					}
				}

				if (change.find(format + "[F][File]", 0) != string::npos)
				{
					change.replace(change.find(format + "[F][File]"), 9 + format.length(), filepath + groupAnimInfo[0]->filename);
					isChange = true;
				}

				if (change.find(format + "[N][File]", 0) != string::npos)
				{
					if (isLastOrder)
					{
						change.replace(change.find(format + "[N][File]"), 9 + format.length(), filepath + filename);
					}
					else
					{
						change.replace(change.find(format + "[N][File]"), 9 + format.length(), filepath + groupAnimInfo[order + 1]->filename);
					}

					isChange = true;
				}

				if (change.find(format + "[L][File]", 0) != string::npos)
				{
					change.replace(change.find(format + "[L][File]"), 9 + format.length(), filepath + groupAnimInfo[lastOrder]->filename);
					isChange = true;
				}

				if (change.find(format + "[", 0) != string::npos)
				{
					string number = boost::regex_replace(string(change), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

					if (change.find(format + "[" + number + "][File]", 0) != string::npos)
					{
						change.replace(change.find(format + "[" + number + "][File]"), 8 + format.length() + number.length(), filepath + groupAnimInfo[stoi(number)]->filename);
						isChange = true;
					}
				}

				if (change.find("File", 0) != string::npos)
				{
					if (filename != change)
					{
						change.replace(change.find("File"), 4, filepath + filename);
						isChange = true;
					}
					else
					{
						cout << "ERROR(1134): Error filename" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
						error = true;
						return;
					}
				}
			}

			// set AnimObject
			if (change.find("&AnimObject", 0) != string::npos)
			{
				vecstr optionInfo = GetOptionInfo(change, format, linecount, lastOrder, groupAnimInfo, false, false, false, order, optionMulti);

				if (error)
				{
					return;
				}

				if (change.find(format + "[", 0) != string::npos)
				{
					if (optionInfo[0] == format && optionInfo[2].find("&AnimObject") && multiAnim)
					{
						int number;
						int groupNum;

						if (optionInfo[1].length() == 0)
						{
							groupNum = animMulti;
						}
						else
						{
							groupNum = stoi(optionInfo[1]);
						}

						if (multiOption.length() != 0 && groupAnimInfo[groupNum]->optionPickedCount[multiOption] > 1)
						{
							number = optionMulti + 1;
						}
						else
						{
							number = groupAnimInfo[groupNum]->AnimObject.begin()->first;
						}

						animObjectReplacer(change, groupAnimInfo[groupNum]->AnimObject, number, format, linecount, true);
						isChange = true;

						if (error)
						{
							return;
						}
					}
				}

				if (change.find("&AnimObject", 0) != string::npos)
				{
					int number;

					if (multiOption.length() != 0 && groupAnimInfo[order]->optionPickedCount[multiOption] > 1)
					{
						number = optionMulti + 1;
					}
					else
					{
						number = groupAnimInfo[order]->AnimObject.begin()->first;
					}

					animObjectReplacer(change, AnimObject, number, format, linecount);
					isChange = true;

					if (error)
					{
						return;
					}
				}
			}

			// get Animation Event Name
			if (change.find("main_anim_event", 0) != string::npos)
			{
				if (change.find(format + "[][main_anim_event]", 0) != string::npos)
				{
					change.replace(change.find(format + "[][main_anim_event]"), 19 + format.length(), groupAnimInfo[animMulti]->mainAnimEvent);
					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[F][main_anim_event]", 0) != string::npos)
				{
					change.replace(change.find(format + "[F][main_anim_event]"), 20 + format.length(), groupAnimInfo[0]->mainAnimEvent);
					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[N][main_anim_event]", 0) != string::npos)
				{
					if (isLastOrder)
					{
						change.replace(change.find(format + "[N][main_anim_event]"), 20 + format.length(), mainAnimEvent);
					}
					else
					{
						change.replace(change.find(format + "[N][main_anim_event]"), 20 + format.length(), groupAnimInfo[order + 1]->mainAnimEvent);
					}

					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find(format + "[L][main_anim_event]", 0) != string::npos)
				{
					change.replace(change.find(format + "[L][main_anim_event]"), 20 + format.length(), groupAnimInfo[lastOrder]->mainAnimEvent);
					isChange = true;

					if (error)
					{
						return;
					}
				}

				if (change.find("main_anim_event", 0) != string::npos)
				{
					change.replace(change.find("main_anim_event"), 15, mainAnimEvent);
					isChange = true;
				}
			}

			if (change.find("[", 0) != string::npos && line.find("]", line.find("[", 0)) != string::npos)
			{
				addOnReplacer(change, addOn, addition, groupAddition, mixOptRegis, optionMulti, masterFormat, linecount);
				isChange = true;
			}

			// get last state
			if (change.find("LastState", 0) != string::npos)
			{
				change.replace(change.find("LastState"), 9, to_string(lastOrder - order + fixedStateID));
				isChange = true;
			}

			if (change.find("eventID[", 0) != string::npos &&  change.find("]", 0) != string::npos)
			{
				eventIDReplacer(change, format, eventid, ZeroEvent, linecount);
				isChange = true;
			}

			if (error)
			{
				error = true;
				return;
			}

			if (change.find("variableID[", 0) != string::npos &&  change.find("]", 0) != string::npos)
			{
				variableIDReplacer(change, format, variableid, linecount);
				isChange = true;
			}

			if (error)
			{
				error = true;
				return;
			}

			if (change.find("import[", 0) != string::npos && change.find("]", 0) != string::npos)
			{
				size_t nextpos = change.find("import[");
				string importer = change.substr(nextpos, change.find("]", change.find("]") + 1) - nextpos + 1);
				__int64 bracketCount = count(importer.begin(), importer.end(), '[');
				__int64 altBracketCount = count(importer.begin(), importer.end(), ']');

				if (IDExist[importer].length() == 0)
				{
					if (bracketCount == 2)
					{
						if (bracketCount == altBracketCount)
						{
							size_t pos = importer.find("[") + 1;
							string file = importer.substr(pos, importer.find("]", pos) - pos);
							pos = importer.find("[", pos) + 1;
							string keyword = importer.substr(pos, importer.find("]", pos) - pos);

							string tempID;

							if ((*newImport)[file][keyword].length() > 0)
							{
								tempID = (*newImport)[file][keyword];
							}
							else
							{
								tempID = strID;
								IDExist[importer] = tempID;
								(*newImport)[file][keyword] = tempID;
								newID();
							}

							change.replace(nextpos, importer.length(), tempID);
							isChange = true;
						}
					}
					else
					{
						cout << "ERROR(1139): Invalid import element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
						error = true;
						return;
					}
				}
				else
				{
					change.replace(nextpos, importer.length(), IDExist[importer]);
					isChange = true;
				}
			}

			if (isChange)
			{
				line.replace(line.find(oldChange), oldChange.length(), change);
			}

			break;
		}
	}
}

void addOnReplacer(string& line, unordered_map<string, vecstr> addOn, ImportContainer addition, unordered_map<string, unordered_map<string, vecstr>> groupAddition, SSMap mixOpt, unsigned int count, string format, int numline)
{
	for (auto it = addOn.begin(); it != addOn.end(); ++it)
	{
		for (unsigned int j = 0; j < it->second.size(); j++)
		{
			if (line.find(it->first + "[" + it->second[j] + "]", 0) != string::npos)
			{
				string add = it->first + "[" + it->second[j] + "]";
				int counter = sameWordCount(line, add);

				for (int i = 0; i < counter; ++i)
				{
					size_t pos = line.find(add, 0);

					if (groupAddition[it->first][it->second[j]].size() == 0 || count == -1)
					{
						if (addition[it->first][it->second[j]].length() == 0)
						{
							cout << "ERROR(1116): Unknown option. Please contact the template creator" << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
							error = true;
							return;
						}

						line.replace(pos, add.length(), addition[it->first][it->second[j]]);
					}
					else
					{
						if (int(groupAddition[it->first][it->second[j]].size()) > count)
						{
							if (groupAddition[it->first][it->second[j]][count].length() == 0)
							{
								cout << "ERROR(1116): Unknown option. Please contact the template creator" << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
								error = true;
								return;
							}

							line.replace(pos, add.length(), groupAddition[it->first][it->second[j]][count]);
						}
						else
						{
							cout << ">> ERROR(1141): BUG FOUND!! Report to Nemesis' author immediately <<" << endl << endl;
							error = true;
							return;
						}
					}
				}
			}

			if (line.find(it->first + "*[" + it->second[j] + "]", 0) != string::npos)
			{
				string add = it->first + "*[" + it->second[j] + "]";
				int counter = sameWordCount(line, add);

				for (int i = 0; i < counter; ++i)
				{
					string option = mixOpt[it->first];
					size_t pos = line.find(add, 0);

					if (groupAddition[option][it->second[j]].size() == 0 || count == -1)
					{
						if (addition[option][it->second[j]].length() == 0)
						{
							cout << "ERROR(1116): Unknown option. Please contact the template creator" << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
							error = true;
							return;
						}

						line.replace(pos, add.length(), addition[option][it->second[j]]);
					}
					else
					{
						if (int(groupAddition[option][it->second[j]].size()) > count)
						{
							if (groupAddition[option][it->second[j]][count].length() == 0)
							{
								cout << "ERROR(1116): Unknown option. Please contact the template creator" << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
								error = true;
								return;
							}

							line.replace(pos, add.length(), groupAddition[option][it->second[j]][count]);
						}
						else
						{
							cout << ">> ERROR(1141): BUG FOUND!! Report to Nemesis' author immediately <<" << endl << endl;
							error = true;
							return;
						}
					}
				}
			}

			if (line.find("[", 0) == string::npos && line.find("]", 0) == string::npos)
			{
				break;
			}
		}

		if (line.find("[", 0) == string::npos && line.find("]", 0) == string::npos)
		{
			break;
		}
	}
}

void animObjectReplacer(string& line, unordered_map<int, string> AnimObject, int id, string format, int linecount, bool otherAnim)
{
	if (otherAnim)
	{
		if (line.find(format + "[&AnimObject/"))
		{
			size_t nextpos = line.find(format + "[&AnimObject/");
			string object = line.substr(nextpos);
			string number = boost::regex_replace(string(object), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
			object = format + "[&AnimObject/" + number + "]";

			if (line.find(object) != string::npos)
			{
				line.replace(nextpos, object.length(), AnimObject[stoi(number)]);
			}
			else
			{
				cout << "ERROR(1108): Invalid AnimObject. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
				error = true;
				return;
			}
		}

		if (line.find(format + "[&AnimObject]"))
		{
			line.replace(line.find(format + "[&AnimObject]"), 13 + format.length(), AnimObject[id]);
		}
	}
	else
	{
		int reference = sameWordCount(line, "&AnimObject/");

		for (int k = 0; k < reference; k++)
		{
			size_t nextpos = line.find("&AnimObject/");
			string object = line.substr(nextpos);
			string number = boost::regex_replace(string(object), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
			object = "&AnimObject/" + number;

			if (line.find(object) != string::npos)
			{
				line.replace(nextpos, object.length(), AnimObject[stoi(number)]);
			}
			else
			{
				cout << "ERROR(1108): Invalid AnimObject. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
				error = true;
				return;
			}
		}

		reference = sameWordCount(line, "&AnimObject");

		if (line.find(format + "&AnimObject"))
		{
			line.replace(line.find("&AnimObject"), 11, AnimObject[id]);
		}
	}
}

void Furniture::stateReplacer(string& line, int stateID, int linecount, string otherAnimOrder, bool otherAnim)
{
	if (otherAnim)
	{
		string number = boost::regex_replace(string(line.substr(line.find("][(S+"))), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
		string state = format + "[" + otherAnimOrder + "][(S+" + number + ")]";

		for (unsigned int i = 0; i < number.size(); ++i)
		{
			if (!isalnum(number[i]) || isalpha(number[i]))
			{
				cout << "ERROR(1153): Invalid character. Only number is acceptable. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "State: " << state << endl << endl;
				error = true;
				return;
			}
		}

		if (line.find(state, 0) != string::npos)
		{
			size_t stateposition = line.find(state, 0);

			if (state == line.substr(stateposition, line.find(")]", stateposition) - stateposition + 2))
			{
				size_t stateLength = state.length();
				line.replace(stateposition, stateLength, to_string(stateID + stoi(number)));
			}
			else
			{
				cout << "ERROR(1137): Invalid state. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "State: " << state << endl << endl;
				error = true;
				return;
			}
		}
	}
	else
	{
		string templine = line.substr(line.find("(S+"));
		string number = boost::regex_replace(string(templine), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));
		string state = "(S+" + number + ")";

		for (unsigned int i = 0; i < number.size(); ++i)
		{
			if (!isalnum(number[i]) || isalpha(number[i]))
			{
				cout << "ERROR(1153): Invalid character. Only number is acceptable. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "State: " << state << endl << endl;
				error = true;
				return;
			}
		}

		if (line.find(state, 0) != string::npos)
		{
			size_t stateposition = line.find(state, 0);

			if (state == line.substr(stateposition, line.find(")", stateposition) - stateposition + 1))
			{
				size_t stateLength = state.length();
				state.replace(1, 1, to_string(stateID));
				calculate(state, error);

				if (error)
				{
					cout << "ERROR(1151): Invalid equation. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "State: " << state << endl << endl;
					error = true;
					return;
				}

				if (stoi(state) >= (*lastState))
				{
					(*lastState) = stoi(state) + 1;
				}

				subFunctionIDs[format + "[" + to_string(order) + "][(S+" + number + ")]"] = state;
				line.replace(stateposition, stateLength, state);
			}
			else
			{
				cout << "ERROR(1137): Invalid state. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << "State: " << templine.substr(0, templine.find(")")) << endl << endl;
				error = true;
				return;
			}
		}
	}
}

string Furniture::addOnConverter(string curline, vecstr elements)
{
	string line = curline;

	addition;

	return line;
}

void eventIDReplacer(string& line, string format, id eventid, string firstEvent, int linecount)
{
	int count = sameWordCount(line, "eventID[");

	for (int i = 0; i < count; ++i)
	{
		size_t nextpos = line.find("eventID[");
		string fullEventName = line.substr(nextpos, line.find("]", nextpos) - nextpos + 1);
		string eventName = fullEventName.substr(8, fullEventName.length() - 9);
		string newEventID = to_string(eventid[eventName]);

		if (newEventID == "0" && eventName != firstEvent)
		{
			if (format == "BASE")
			{
				cout << ">> ERROR(1166): BUG FOUND!! Report to Nemesis' author immediately <<" << endl << "Line: " << linecount << endl << endl;
				error = true;
				return;
			}
			else
			{
				cout << "ERROR(1131): Non-registrated event ID detected. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
				error = true;
				return;
			}
		}

		line.replace(line.find(fullEventName), fullEventName.length(), newEventID);
	}
}

void variableIDReplacer(string& line, string format, id variableid, int linecount)
{
	int count = sameWordCount(line, "variableID[");

	for (int i = 0; i < count; ++i)
	{
		size_t nextpos = line.find("variableID[");
		string varName = line.substr(nextpos, line.find("]", nextpos) - nextpos + 2);
		string newVarID = to_string(variableid[varName.substr(11, varName.length() - 12)]);

		if (newVarID == "0")
		{
			if (format == "BASE")
			{
				cout << ">> ERROR(1166): BUG FOUND!! Report to Nemesis' author immediately <<" << endl << "Line: " << linecount << endl << endl;
				error = true;
				return;
			}
			else
			{
				cout << "ERROR(1132): Non-registrated variable ID detected. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << linecount << endl << endl;
				error = true;
				return;
			}
		}

		line.replace(line.find(varName), varName.length(), newVarID);
	}
}

vecstr GetOptionInfo(string line, string format, int numline, size_t lastOrder, vector<shared_ptr<animationInfo>> groupAnimInfo, bool allowNoFixAnim, bool isCondition, bool isGroup, int animMulti, int optionMulti)
{
	int limiter;
	vecstr optionInfo;

	if (isCondition)
	{
		limiter = 3;
	}
	else
	{
		limiter = 4;
	}

	optionInfo.reserve(limiter);

	if (line.find(format + "[") != string::npos && groupAnimInfo.size() != 0)
	{
		if (sameWordCount(line, format + "[") > 1)
		{
			cout << "ERROR(1157): Function call within a function call is not supported. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Function call: " << line << endl << endl;
			error = true;
			return optionInfo;
		}

		string templine = line;
		size_t pos = templine.find(format + "[");
		optionInfo.push_back(templine.substr(pos, format.length()));
		templine = templine.substr(templine.find("[", pos) + 1);

		while (true)
		{
			pos = templine.find("]");
			optionInfo.push_back(templine.substr(0, pos));
			size_t optionLength = optionInfo.back().length() + 1;

			if (templine.length() > optionLength && templine[optionLength] == '[')
			{
				templine = templine.substr(templine.find("[") + 1);
			}
			else
			{
				break;
			}
		}

		if (optionInfo[1] == "F")
		{
			optionInfo[1] = "0";
		}
		else if (optionInfo[1] == "L")
		{
			optionInfo[1] = to_string(lastOrder);
		}
		else if (optionInfo[1] == "N")
		{
			if (!isGroup)
			{
				optionInfo[1] = to_string(animMulti + 1);
			}
			else
			{
				cout << "ERROR(1052): Invalid element. Only empty, \"F\", \"L\" or number is acceptable for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
				error = true;
				return optionInfo;
			}
		}
		else if (optionInfo[1].length() == 0)
		{
			if (!allowNoFixAnim)
			{
				if (!isGroup)
				{
					cout << "ERROR(1057): Invalid element. Only \"F\", \"N\", \"L\" or number is acceptable for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
				}
				else
				{
					cout << "ERROR(1059): Invalid element. Only \"F\", \"L\" or number is acceptable for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
				}

				error = true;
				return optionInfo;
			}
			else
			{
				if (animMulti == -1)
				{
					if (!isCondition)
					{
						if (!isGroup)
						{
							cout << "ERROR(1057): Invalid element. Only \"F\", \"N\", \"L\" or number is acceptable for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
						}
						else
						{
							cout << "ERROR(1059): Invalid element. Only \"F\", \"L\" or number is acceptable for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
						}

						error = true;
						return optionInfo;
					}
				}
				else
				{
					optionInfo[1] = to_string(animMulti);
				}
			}
		}
		else
		{
			templine = optionInfo[1];
			templine = templine + "a";
			string newtempline = boost::regex_replace(string(templine), boost::regex("[^0-9]*([0-9]+).*"), string("\\1"));

			if (newtempline == templine)
			{
				cout << "ERROR(1055): Invalid element for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
				error = true;
				return optionInfo;
			}

			if (stoi(optionInfo[1]) > lastOrder)
			{
				cout << "ERROR(1148): \"Minimum\" in option_list.txt must be used and contain larger value than the 1st element being used. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
				error = true;
				return optionInfo;
			}
		}			

		if (optionInfo[2][optionInfo[2].length() - 1] == '*')
		{
			string option = optionInfo[2].substr(0, optionInfo[2].length() - 1);

			if (groupAnimInfo[stoi(optionInfo[1])]->mixOptRegis[option].length() != 0)
			{
				optionInfo[2] = groupAnimInfo[stoi(optionInfo[1])]->mixOptRegis[option];
			}
			else
			{
				cout << "ERROR(1109): Invalid link. The said option is not linked to any other option. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
				error = true;
				return optionInfo;
			}
		}

		if (!isCondition && optionInfo.size() > 3)
		{
			if (optionInfo.size() == 5)
			{
				bool isPassed = true;

				if (optionInfo[4].length() == 0)
				{
					if (optionMulti == -1)
					{
						cout << "ERROR(1060): Invalid element. Specific element is required for the 3rd element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
						error = true;
						return optionInfo;
					}
					else
					{
						optionInfo[4] = to_string(optionMulti);
					}
				}
				else
				{
					for (unsigned int i = 0; i < optionInfo[4].size(); ++i)
					{
						if (!isalnum(optionInfo[4][i]) || isalpha(optionInfo[4][i]))
						{
							cout << "ERROR(1055): Invalid element for the 1st element. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
							error = true;
							return optionInfo;
						}
					}
				}
			}

			if (groupAnimInfo[stoi(optionInfo[1])]->groupAddition[optionInfo[2]][optionInfo[3]].size() != 0)
			{
				limiter++;
			}
		}
		
		if (optionInfo.size() > 2 && (optionInfo[2] == "main_anim_event" || optionInfo[2] == "StateID" || optionInfo[2] == "File" || optionInfo[2].find("AnimObject") != string::npos || optionInfo[2].find("(S+") != string::npos))
		{
			limiter--;
		}

		if (int(optionInfo.size()) > limiter)
		{
			cout << "ERROR(1054): Invalid option. Only " << limiter - 1 << " elements are required excluding the header. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
			error = true;
			return optionInfo;
		}
		else if (int(optionInfo.size()) < limiter)
		{
			cout << "ERROR(1054): Invalid option. " << limiter - 1 << " elements are required excluding the header. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
			error = true;
			return optionInfo;
		}

		if (optionInfo[2][0] == '^' && isGroup)
		{
			cout << "ERROR(1149): Invalid condition. Order condition cannot be used in group template. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
			error = true;
			return optionInfo;
		}

		if (optionInfo[0] != format)
		{
			cout << "ERROR(1051): Invalid header. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
			error = true;
		}
	}
	else
	{
		if (isGroup)
		{
			cout << "ERROR(1152): Missing animation specification. Use Special Condition(<format>[<first element>][<second element>]) instead. Please contact the template creator" << endl << "Template: " << format + "_group" << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
			error = true;
		}
		else
		{
			optionInfo.push_back(format);
			optionInfo.push_back(to_string(animMulti));

			if (line[line.length() - 1] == '*')
			{
				string option = line.substr(0, line.length() - 1);

				if (groupAnimInfo[stoi(optionInfo[1])]->mixOptRegis[option].length() != 0)
				{
					optionInfo.push_back(groupAnimInfo[stoi(optionInfo[1])]->mixOptRegis[option]);
				}
				else
				{
					cout << "ERROR(1109): Invalid link. The said option is not linked to any other option. Please contact the template creator" << endl << "Template: " << format << endl << "Line: " << numline << endl << "Option: " << line << endl << endl;
					error = true;
					return optionInfo;
				}
			}
			else
			{
				optionInfo.push_back(line);
			}
		}
	}

	return optionInfo;
}

template <class T>
inline int numDigits(T number)
{
	int digits = 0;

	if (number < 0)
	{
		digits = 1; // remove this line if '-' counts as a digit
	}

	while (number)
	{
		number /= 10;
		digits++;
	}

	return digits;
}

string getOption(string curline, bool multi)
{
	if (!multi)
	{
		size_t nextpoint = curline.find("<!-- NEW ^") + 10;
		return curline.substr(nextpoint, curline.find("^ -->", nextpoint) - nextpoint);
	}
	else
	{
		size_t nextpoint = curline.find("<!-- NEW ^") + 10;
		return curline.substr(nextpoint, curline.find("^ +% -->", nextpoint) - nextpoint);
	}
}

inline int sameWordCount(string line, string word)
{
	size_t nextWord = -1;
	int wordCount = 0;

	while (true)
	{
		nextWord = line.find(word, nextWord + 1);

		if (nextWord != -1)
		{
			wordCount++;
		}
		else
		{
			break;
		}
	}

	return wordCount;
}

inline bool isPassed(int condition, unordered_map<int, bool> IsConditionOpened)
{

	for (int k = condition - 1; k > 0; k--)
	{
		if (!IsConditionOpened[k])
		{
			return false;
		}
	}

	return true;
}