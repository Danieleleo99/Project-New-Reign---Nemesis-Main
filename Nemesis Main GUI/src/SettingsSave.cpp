#include <fstream>
#include "SettingsSave.h"
#include "Global.h"
#include "alphanum.hpp"
#include "ErrorMsgBox.h"
#include "functions\writetextfile.h"

void createLanguageCache(std::string language)
{
	if (CreateFolder("cache"))
	{
		FileWriter cachefile("cache\\language setting");

		if (cachefile.is_open())
		{
			cachefile << language;
		}
		else
		{
			CEMsgBox* msg = new CEMsgBox;
			msg->setWindowTitle("ERROR");
			msg->setText("Error: Failed to create cache file. Please ensure Nemesis has permission to create file in current folder");
			msg->show();
		}
	}
}

void createModCache(std::vector<std::string> chosenBehavior)
{
	if (CreateFolder("cache"))
	{
		FileWriter cachefile("cache\\mod settings");

		if (cachefile.is_open())
		{
			for (auto& line : chosenBehavior)
			{
				cachefile << line + "\n";
			}
		}
		else
		{
			CEMsgBox* msg = new CEMsgBox;
			msg->setWindowTitle("ERROR");
			msg->setText("Error: Failed to create cache file. Please ensure Nemesis has permission to create file in current folder");
			msg->show();
		}
	}
}

void createModOrderCache(std::vector<std::string> behaviorList)
{
	if (CreateFolder("cache"))
	{
		FileWriter cachefile("cache\\order list");

		if (cachefile.is_open())
		{
			for (auto& line : behaviorList)
			{
				cachefile << line + "\n";
			}
		}
		else
		{
			CEMsgBox* msg = new CEMsgBox;
			msg->setWindowTitle("ERROR");
			msg->setText("Error: Failed to create cache file. Please ensure Nemesis has permission to create file in current folder");
			msg->show();
		}
	}
}

bool getCache(std::string& language, std::unordered_map<std::string, bool>& chosenBehavior)
{
	if (!isFileExist("cache"))
	{
		CreateFolder("cache");
		return false;
	}

	std::string filename = "cache\\language setting";
	vecstr storeline;

	if (!isFileExist(filename) || !GetFunctionLines(filename, storeline, false))
	{
		language = "english";
	}
	else
	{
		for (auto& line : storeline)
		{
			if (line.length() > 0)
			{
				language = line;
			}
		}

		if (language.length() == 0)
		{
			QMessageBox* msg = new QMessageBox;
			msg->setWindowTitle("WARNING");
			msg->setText("Warning: Failed to read language cache file. Language is set to the default language (english)");
			msg->setAttribute(Qt::WA_DeleteOnClose);
			msg->setIcon(QMessageBox::Warning);
			msg->show();
			language = "english";
		}
	}

	filename = "cache\\mod settings";

	if (!isFileExist(filename))
	{
		return false;
	}

	if (!GetFunctionLines(filename, storeline, false))
	{
		QMessageBox* msg = new QMessageBox;
		msg->setWindowTitle("WARNING");
		msg->setText("Warning: Failed to read mod cache file. All mods will be reverted to unchecked state");
		msg->setAttribute(Qt::WA_DeleteOnClose);
		msg->setIcon(QMessageBox::Warning);
		msg->show();
		return false;
	}

	for (auto& line : storeline)
	{
		if (line.length() > 0)
		{
			chosenBehavior[line] = true;
		}
	}

	if (chosenBehavior.size() > 0)
	{
		return true;
	}

	return false;
}

bool getCache(std::wstring& language, std::unordered_map<std::string, bool>& chosenBehavior)
{
	if (!isFileExist("cache"))
	{
		CreateFolder("cache");
		return false;
	}

	std::wstring filename = L"cache\\language setting";
	std::vector<std::wstring> storeline;

	if (!isFileExist(filename) || !GetFunctionLines(filename, storeline, false))
	{
		language = L"english";
	}
	else
	{
		for (auto& line : storeline)
		{
			if (line.length() > 0)
			{
				language = line;
			}
		}

		if (language.length() == 0)
		{
			QMessageBox* msg = new QMessageBox;
			msg->setWindowTitle("WARNING");
			msg->setText("Warning: Failed to read language cache file. Language is set to the default language (english)");
			msg->setAttribute(Qt::WA_DeleteOnClose);
			msg->setIcon(QMessageBox::Warning);
			msg->show();
			language = L"english";
		}
	}

	filename = L"cache\\mod settings";

	if (!isFileExist(filename))
	{
		return false;
	}

	if (!GetFunctionLines(filename, storeline, false))
	{
		QMessageBox* msg = new QMessageBox;
		msg->setWindowTitle("WARNING");
		msg->setText("Warning: Failed to read mod cache file. All mods will be reverted to unchecked state");
		msg->setAttribute(Qt::WA_DeleteOnClose);
		msg->setIcon(QMessageBox::Warning);
		msg->show();
		return false;
	}

	for (auto& line : storeline)
	{
		if (line.length() > 0)
		{
			chosenBehavior[WStringToString(line)] = true;
		}
	}

	if (chosenBehavior.size() > 0)
	{
		return true;
	}

	return false;
}

bool getOrderCache(std::vector<std::string>& orderList)
{
	if (!isFileExist("cache"))
	{
		CreateFolder("cache");
		return false;
	}

	std::string filename = "cache\\order list";
	std::vector<std::string> storeline;

	if (!isFileExist(filename))
	{
		return false;
	}

	if (!GetFunctionLines(filename, storeline, false))
	{
		return false;
	}

	orderList.clear();
	orderList.reserve(storeline.size());

	for (auto& line : storeline)
	{
		if (line.length() > 0)
		{
			orderList.push_back(line);
		}
	}

	return orderList.size() > 0;
}
