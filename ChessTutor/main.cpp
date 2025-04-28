/*
 * main.cpp
 *
 * Entry point for the Chess Tutor application. Initializes the Qt
 * application object, creates the MainWindow UI, and starts the
 * event loop.
 *
 * @author  ESL Team
 * @date    2025-04-22
 */
#include "mainwindow.h"
#include <QApplication>

/**
 * main
 *
 * Constructs the QApplication, creates and shows the main window,
 * then enters the Qt event loop.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return Exit code from the Qt event loop
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
