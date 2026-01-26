#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>
#include <QListWidgetItem>
#include <QFont>
#include <QIcon>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <QSettings>
#include <QDebug>
#include <QTextStream>
#include <QUuid>
#include <QString>
#include <fstream>
#include <string>
#include <objbase.h>


using namespace std;

class tools
{
public:
    tools();
    QString getDriveLetter();
};

std::string guidToString(const GUID& guid);





#endif // TOOLS_H
