#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <stdio.h>
int main(int argc, char **argv){
	QApplication app = QApplication(argc,argv);
	QWidget *window = new QWidget();

	QPushButton *button1 = new QPushButton("app1",window);
	QPushButton *button2 = new QPushButton("app2",window);

	QGridLayout *grid = new QGridLayout(window);
	grid->addWidget(button1,0,0);
	grid->addWidget(button2,0,1);

	window->show();
	int status = app.exec();
	return status;
}
