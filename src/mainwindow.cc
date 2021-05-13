#include "../include/mainwindow.h"

#include <QtWidgets>

#define THREADS 8

constexpr auto NAME{ "Name" };
constexpr auto COLORS{ "Colors" };
constexpr auto GENERAL{ "General" };
constexpr auto LOG_LEVEL{ "LogLevel" };
constexpr auto DATASET{ "Dataset" };


MainWindow::MainWindow()
{
	Logger->trace("MainWindow::MainWindow()");
}

MainWindow::MainWindow(QJsonObject const& a_config)
{
	Logger->trace("MainWindow::MainWindow()");
	MainWindow::configure(a_config);
}

void MainWindow::configure(QJsonObject const& a_config) 
{
	QJsonObject jColors = a_config[COLORS].toObject();
	QJsonObject jDataset = a_config[DATASET].toObject();

	createMenus();
	setupView(a_config);
	setupLayout();
	setupMainWidget();
}

void MainWindow::setupMainWidget() 
{
	Logger->trace("MainWindow::MainWindow() mainCentralLayout:");
	QGridLayout* mainCentralLayout = new QGridLayout;
	mainCentralLayout->setContentsMargins(0, 0, 0, 0);
	//mainCentralLayout->addWidget(leftToolBar, 0, 0);
	mainCentralLayout->addWidget(m_rightWidget, 0, 1);

	QWidget* mainCentralWidget = new QWidget(this);
	mainCentralWidget->setLayout(mainCentralLayout);

	m_progressBar = new QProgressBar(this);
	m_progressBar->setMinimum(0);
	m_progressBar->setMaximum(100);

	Logger->trace("MainWindow::MainWindow() mainLayout:");
	QGridLayout* mainLayout = new QGridLayout;
	m_view->setMinimumWidth(1000);
	
	mainLayout->setContentsMargins(0, 0, 0, 0);
	//mainLayout->addWidget(menuBar, 0, 0);
	mainLayout->addWidget(m_view, 0, 0);
	mainLayout->addWidget(mainCentralWidget, 1, 0);
	mainLayout->addWidget(m_progressBar, 2, 0);
	
	setLayout(mainLayout);

	resize(1600, 600);
}

void MainWindow::createMenus()
{
	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu(tr("&File"));
	//m_fileMenu->addAction(action_open);
	m_fileMenu->addSeparator();
	//m_fileMenu->addAction(action_autoSaving);
}

void MainWindow::setupView(QJsonObject const& a_config)
{
	m_view = new View(a_config);
	m_view->setMinimumWidth(1000);
/*

	connect(this, &MainWindow::addImageToScene, view, &View::onAddImageToScene);
	connect(this, &MainWindow::setRectOpacity, view, &View::onSetRectOpacity);
	connect(this, &MainWindow::renderColors, view, &View::renderColorsFromJson);
	connect(this, &MainWindow::addRectToScene, view, &View::onAddRectToScene);
	connect(this, &MainWindow::setColor, view, &View::setColor);
	connect(this, &MainWindow::updateFilename, view, &View::onUpdateFilename);
	connect(this, &MainWindow::addList, this, &MainWindow::onAddList);
	connect(this, &MainWindow::clearItems, view, &View::onClearItems);

	connect(view, &View::updateLabelList, this, &MainWindow::onUpdateLabelList);
	connect(view, &View::updateFile, this, &MainWindow::onUpdateFile);
	connect(view, &View::deleteList, this, &MainWindow::onDeleteList);
	connect(view, &View::addList, this, &MainWindow::onAddList);
	*/
}



void MainWindow::setupLayout() 
{
	spdlog::trace("MainWindow::MainWindow() setupRightLayout");
	m_labelList = new QTreeView();
	//m_labelList->setModel(modelList);
	m_labelList->setRootIsDecorated(true);
	m_labelList->setAlternatingRowColors(true);
	m_labelList->setSortingEnabled(true);
	m_labelList->setAnimated(true);
	m_labelList->setSelectionMode(QAbstractItemView::SingleSelection);

	m_rightLayout = new QVBoxLayout;
	m_rightLayout->setContentsMargins(0, 0, 0, 0);
	//m_rightLayout->addWidget(m_buttonContainer);
	m_rightLayout->addWidget(m_labelList);
	//m_rightLayout->addWidget(proxyView);
	m_rightLayoutContainer = new QWidget;
	m_rightLayoutContainer->setLayout(m_rightLayout);
	m_gridLayout = new QGridLayout(this);
	
	m_gridLayout->addWidget(m_view, 0, 0);
	m_gridLayout->addWidget(m_rightLayoutContainer, 0, 1);
	m_rightWidget = new QWidget(this);
	m_rightWidget->setLayout(m_gridLayout);
}



	
