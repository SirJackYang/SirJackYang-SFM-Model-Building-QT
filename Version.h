#include <QString>

#define POINTLAB_VER_NUM "2.0"
#define POINTLAB_SUB_VER "beta"

static QString GetPointLabVersion()
{
	QString verStr = QString("%1.%2").arg(POINTLAB_VER_NUM).arg(POINTLAB_SUB_VER);

#if defined(_WIN64)
	QString arch = "64 bits";
#else
	QString arch = "32 bits";
#endif

	verStr += QString(" [%1]").arg(arch);

#ifdef _DEBUG
	verStr += QString(" [DEBUG]");
#endif // _DEBUG

	return verStr;
}
