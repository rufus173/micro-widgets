#ifndef _MAIN_H
#define _MAIN_H
#include <QLineEdit>
#include <QObject>
class CustomLineEdit : public QLineEdit{
	Q_OBJECT
	public:
	//CustomLineEdit(QWidget *parent = nullptr);
	bool event(QEvent *event);
	signals:
	void escapePressed();
};
#endif
