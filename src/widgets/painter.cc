#include "widgets/painter.h"

constexpr auto ROI{ "ROI" };
constexpr auto NAME{ "Name" };
constexpr auto WIDTH{ "Width" };
constexpr auto HEIGHT{ "Height" };
constexpr auto X{ "X" };
constexpr auto Y{ "Y" };
constexpr auto SIZE{ "Size" };


Painter::Painter(QJsonObject const& config, GraphicsScene *graphicsScene, GraphicsView* graphicsView)
	: m_config(config)
	, m_graphicsScene(graphicsScene)
	, m_graphicsView(graphicsView)
{
	#ifdef _WIN32
	m_split = "\\";
	#endif // _WIN32
	#ifdef __linux__
	m_split = "/";
	#endif // _UNIX
	m_roiType = 4;
	m_paintType = 5;
	m_imageType = 6;
    m_painterSettings.configureColors(m_config);
	m_graphicsView->setPainterSettings(&m_painterSettings);
}

void Painter::onPaintOnBoard(qint32 x, qint32 y)
{
	Logger->debug("Painter::onPaintOnBoard()");
	if (m_painterSettings.m_penSize == 1)
	{
		m_paintImage.setPixelColor(x, y, m_painterSettings.m_color);
	}
	else if (m_painterSettings.m_penSize > 1 && m_painterSettings.m_penSize < 20)
	{
		qint32 it = m_painterSettings.m_penSize - 1;
		for (int zx = -it; zx <= it; zx++)
		{
			for (int zy = -it; zy <= it; zy++)
			{
				m_paintImage.setPixelColor(x + zx, y + zy, m_painterSettings.m_color);
			}
		}
	}
	else
	{
		Logger->error("Painter::onPaintOnBoard() painter not select");
	}
	m_paintPixmap->setPixmap(QPixmap::fromImage(m_paintImage));
}

void Painter::onLoadImage(QString dir, QString name)
{
	m_name = name;
	QString m_fileNameWithPath = dir + m_split + name;
	Painter::clearScene();
	Logger->debug("Painter::onLoadImage({})",m_fileNameWithPath.toStdString());
	QPixmap test;
	test.load(m_fileNameWithPath);
	addImageToScene(test);
}

void Painter::onLoadRois(QString dir, QString name)
{
	QString _fileNameWithPath = dir + m_split + name + ".json";
	Logger->debug("ImageLoader::loadRois() _fileNameWithPath:{}", _fileNameWithPath.toStdString());

	std::shared_ptr<ConfigReader> cR = std::make_shared<ConfigReader>();

	QJsonObject jObject;
	if (!cR->readConfig(_fileNameWithPath, jObject))
	{
		spdlog::error("File {} read confif failed", _fileNameWithPath.toStdString());
	}

	QJsonArray jROI = jObject[ROI].toArray();

	Logger->debug("ImageLoader::loadRois() jROI.size():{}", jROI.size());
	Painter::addRoisToScene(jROI);
}

void Painter::clearScene()
{
	QList<QGraphicsItem*> items = m_graphicsScene->items(Qt::DescendingOrder);
	Logger->trace("Painter::clearScene() size:{}", items.size());
	for (int i = 0; i < items.size(); i++)
	{
		Logger->trace("Painter::clearScene() type:{}", items[i]->type());
		if (items[i]->type() == m_imageType || items[i]->type() == m_paintType)
		{
			GraphicsPixmapItem* cast = dynamic_cast<GraphicsPixmapItem*>(items[i]);
			m_graphicsScene->removeItem(cast);
			delete cast;
		}
	}
	m_graphicsScene->clear();
}

static cv::Mat qimage_to_mat_ref(QImage &img, int format)
{
    return cv::Mat(img.height(), img.width(), format, img.bits(), img.bytesPerLine());
}

void Painter::deleteRois()
{
	QList<QGraphicsItem*> items = m_graphicsScene->items(Qt::DescendingOrder);
	Logger->trace("Painter::onItemChanged() size:{}", items.size());
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i]->type() == m_roiType) // Just ROI object:
		{
			GraphicsRectItem* cast = dynamic_cast<GraphicsRectItem*>(items[i]);
			m_graphicsScene->removeItem(cast);
			delete cast;
		}
	}
}

void Painter::onSetCurrentPaintFolder(QString imageFolder, QString paintFolder, QString jsonDirectory)
{
	m_currentDirectory = imageFolder;
	m_currentPaintDirectory = paintFolder;
	m_currentJsonDirectory = jsonDirectory;
}

void Painter::onSaveRois(QString dir, QString name)
{
	Logger->trace("Painter::onSaveRois({}, {})", dir.toStdString(), name.toStdString());
	QString m_fileNameWithPath = dir + m_split + name + ".json";

	QJsonArray _ROIArray;
	std::vector<QJsonArray> _arrays;

	for (qint32 color = 0; color < m_painterSettings.m_colors.size(); color++)
	{
		//_counter.push_back(0);
		_arrays.push_back(QJsonArray());
	}

	QList<QGraphicsItem*> items = m_graphicsScene->items(Qt::DescendingOrder);
	spdlog::trace("View::onSavewhiteBoard() size:{}", items.size());
	for (int i = 0; i < items.size(); i++)
	{
		QString filename = "";
		QRectF rect = items[i]->boundingRect();
		QPointF pos = items[i]->pos();

		int x = rect.x() + 1;
		int y = rect.y() + 1;
		int width = rect.width() - 1;
		int height = rect.height() - 1;
		int sizeRoi = qAbs(width / 2) * qAbs(height / 2);

		QSize size = QSize(width, height);
		spdlog::trace("items size:{}x{}x{}x{}", x, y, size.width(), size.height());

		if (items[i]->type() == m_roiType)
		{
			GraphicsRectItem* cast = dynamic_cast<GraphicsRectItem*>(items[i]);
			bool b_saveFlag = true;
			//QString nameOfRect = cast->text();
			for (int i = 0 ; i < m_painterSettings.m_colors_background.size() ; i++)
			{	
				if (cast->text() == m_painterSettings.m_colors_background[i])
				{
					b_saveFlag = false;
				}
			}
			if(b_saveFlag)
			{

				QJsonObject obj{ { X, x },
					{ Y, y },
					{ WIDTH, width },
					{ HEIGHT, height },
					{ NAME, cast->text() },
					{ SIZE, sizeRoi } };
				_ROIArray.append(obj);
			}
		}
	}
	qDebug() << "_ROIArray:" << _ROIArray;

	QJsonObject json{};
	json.insert(ROI, _ROIArray);
	auto test = QJsonDocument(json).toJson(QJsonDocument::Indented);
	//QString nameBest = m_path + m_filename + m_prefix;
	QFile jsonFile(QString::fromStdString(m_fileNameWithPath.toStdString()));
	jsonFile.open(QFile::WriteOnly);
	jsonFile.write(test);
	jsonFile.close();

	QString possibleError;
	QJsonArray jROI = json[ROI].toArray();
	qDebug() << "jROI:" << jROI;
}


void Painter::onSavePaint(QString dir, QString name)
{
	Logger->trace("Painter::onSavePaint({}, {})", dir.toStdString(), name.toStdString());
	QString m_fileNameWithPath = dir + m_split + name + ".png";
	cv::Mat cleanData = cv::Mat(m_paintImage.height(), m_paintImage.width(), CV_8UC1, cv::Scalar(0));

	int counter;

	qint32 _counterWhite = 0;
	qint32 _counterShadow = 0;
	for (int i = 0; i < m_painterSettings.m_colors.size(); i++)
	{
		QString color = m_painterSettings.m_colors[i];
		for (int i = 0; i < m_paintImage.width(); i++)
		{
			for (int j = 0; j < m_paintImage.height(); j++)
			{
				QGraphicsItem* item = m_graphicsScene->itemAt(QPointF(i, j), QTransform());
				if (item->type() == m_roiType)
				{
					if (m_painterSettings.m_colorHash[color].rgb() == m_paintImage.pixel(i, j))
					{
						counter++;
						cleanData.at<unsigned char>(j,i) = m_painterSettings.m_colorIntHash[color];
					}
				}
			}
		}
	}

	cv::imwrite(m_fileNameWithPath.toStdString(), cleanData);
	Logger->trace("View::onSavePaint(): m_fileNameWithPath:{}", m_fileNameWithPath.toStdString());
}

void Painter::onCreateRois()
{
	Painter::deleteRois();
	QJsonArray _array;
	Logger->trace("Painter::onCreateRois()");
	for (int color = 0; color < m_painterSettings.m_colors_foreground.size(); color++)
	{
		Logger->trace("Painter::onCreateRois() color:{}", color);
		qDebug() <<m_painterSettings.m_colors_foreground[color];
		qDebug() <<m_painterSettings.m_colorHash[m_painterSettings.m_colors_foreground[color]];

		cv::Mat cleanData = cv::Mat(m_paintImage.height(), m_paintImage.width(), CV_8UC1,cv::Scalar(0));
		for (int i = 0; i < m_paintImage.width(); i++)
		{
			for (int j = 0; j < m_paintImage.height(); j++)
			{
				if ( m_painterSettings.m_colorHash[m_painterSettings.m_colors_foreground[color]].rgb() == m_paintImage.pixel(i,j))
				{
					cleanData.at<unsigned char>(j,i) = 255;
				}
			}
		}
		QJsonArray contoursArray{};
		m_contour.CrateRois(cleanData, m_painterSettings.m_colors_foreground[color], contoursArray);
		for(int i = 0 ; i < contoursArray.size() ; i++)
		{
			_array.append(contoursArray[i]);
		}

		Logger->trace("Painter::addRoisToScene()");
		QString _name = QString::number(color) + "_CrateRois.png";
		cv::imwrite(_name.toStdString(), cleanData);
		
	}
	Painter::addRoisToScene(_array);
}

void Painter::addRoisToScene(QJsonArray contoursArray)
{
	Logger->warn("contoursArray.size:{}", contoursArray.size());
	emit(clearList());

	for (unsigned int i = 0; i < contoursArray.size(); i++)
	{
		QJsonObject obj = contoursArray[i].toObject();
		QString name = obj[NAME].toString();
		int size = obj[SIZE].toInt();
		QRectF tempRectToText = QRectF(obj[X].toInt(), obj[Y].toInt(), obj[WIDTH].toInt(),obj[HEIGHT].toInt());
		qDebug() << "tempRectToText:" << tempRectToText;
		QColor _color = m_painterSettings.m_colorHash[name];
		Logger->warn("Painter::addRoisToScene()  add GraphicsRectItem:{} name:{}", i, name.toStdString());
		qDebug() << "color:" << _color;
		GraphicsRectItem * itemG = new GraphicsRectItem(_color, name, tempRectToText, m_roiType);
		m_graphicsScene->addItem(itemG);
		emit(addList(i, name, size, true));
	}
}

void Painter::addImageToScene(QPixmap image)
{
	Logger->trace("Painter::addImageToScene()");
	m_pixmap = static_cast<GraphicsPixmapItem*>(m_graphicsScene->addPixmap(image));
	
	//m_pixmap = new GraphicsPixmapItem();
	m_pixmap->configure(m_imageType);
	Logger->trace("Painter::addImageToScene() m_pixmap.type:{}",m_pixmap->type());
	m_pixmap->setEnabled(true);
	m_pixmap->setVisible(true);
	m_pixmap->setOpacity(1.0);
	m_pixmap->setAcceptHoverEvents(true);
	m_pixmap->setAcceptTouchEvents(true);
	m_pixmap->setZValue(-2);
	//m_pixmap->update();
	//m_graphicsScene->addPixmap(image);
	m_pixmap->update();
	Logger->trace("Painter::addImageToScene() before configure: m_pixmap.type:{}",m_pixmap->type());

	m_image = image.toImage();
	m_paintImage = image.toImage();

	for (int y = 0; y < m_paintImage.height(); y++)
	{
		for (int x = 0; x < m_paintImage.width(); x++)
		{
			m_paintImage.setPixelColor(x, y, QColor{ 255, 255, 255, 127 });
		}
	}
	
	QPixmap whiteBoardPixmap = QPixmap::fromImage(m_paintImage);
	m_paintPixmap = static_cast<GraphicsPixmapItem*>(m_graphicsScene->addPixmap(whiteBoardPixmap));
	//m_paintPixmap = new GraphicsPixmapItem();
	m_paintPixmap->configure(m_paintType);
	Logger->trace("Painter::addImageToScene() m_paintPixmap.type:{}",m_paintPixmap->type());
	m_paintPixmap->setEnabled(true);
	m_paintPixmap->setVisible(true);
	m_paintPixmap->setOpacity(0.5);
	m_paintPixmap->setAcceptHoverEvents(true);
	m_paintPixmap->setAcceptTouchEvents(true);
	m_paintPixmap->setZValue(-1);
	//m_graphicsScene->addPixmap(whiteBoardPixmap);
	m_paintPixmap->update();
	Logger->trace("Painter::addImageToScene() before configure: m_paintPixmap.type:{}",m_paintPixmap->type());
	
	emit(updateView());
}

void Painter::onLoadPaint(QString dir, QString name)
{
	QString _fileNameWithPath = dir + m_split + name + ".png";
	Logger->debug("ImageLoader::onLoadPaint() _fileNameWithPath:{}", _fileNameWithPath.toStdString());

	cv::Mat image = cv::imread(_fileNameWithPath.toStdString());
	
	Logger->trace("Painter::onLoadPaint() image ({}x{}x{})", image.cols, image.rows, image.channels());
	if(image.empty())
	{
		Logger->error("Painter::onLoadPaint() image cant be loaded:{}", _fileNameWithPath.toStdString());
		return;
	}
	else
	{
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
		for (qint32 color = 0; color < m_painterSettings.m_colors.size(); color++)
		{
			for (int i = 0; i < image.cols; i++)
			{
				for (int j = 0; j < image.rows; j++)
				{
					if ( m_painterSettings.m_colorIntHash[m_painterSettings.m_colors[color]] == image.at<unsigned char>(j, i))
					{
						onPaintColors(i, j, m_painterSettings.m_colorHash[m_painterSettings.m_colors[color]]);
					}
				}
			}
		}
	}
	Painter::onPaintColorsFinish();
}

void Painter::onPaintColors(qint32 x, qint32 y, QColor color)
{
	//Logger->trace("Painter::onPaintColors()");
	m_paintImage.setPixelColor(x, y, color);
}

void Painter::onPaintColorsFinish()
{
	Logger->trace("Painter::onPaintColorsFinish()");
	m_paintPixmap->setPixmap(QPixmap::fromImage(m_paintImage));
	emit(updateView());
}

void Painter::onChangeColor(QColor color)
{
	qDebug() << "Painter::onChangeColor:" << color;
	m_painterSettings.m_color = color;
}

void Painter::onChangeOldColor(QString name, QColor color)
{
	qDebug() << "Painter::onChangeOldColor:" << color;
	// Check if color exist:
	for (qint32 i = 0; i < m_painterSettings.m_colors.size(); i++)
	{
		qDebug() << "_painterSettings.m_colorHash[m_painterSettings.m_colors[i]]" << m_painterSettings.m_colorHash[m_painterSettings.m_colors[i]];
		qDebug() << "color" << color;
		if(m_painterSettings.m_colorHash[m_painterSettings.m_colors[i]].rgb() == color.rgb())
		{
			return;
		}
	}
	
	QColor oldColor = m_painterSettings.m_colorHash[name];

	for (int i = 0; i < m_paintImage.height(); i++)
	{
		for (int j = 0; j < m_paintImage.width(); j++)
		{
			if (m_paintImage.pixel(j, i) == oldColor.rgb())
			{
				m_paintImage.setPixelColor(j, i, color);
			}
		}
	}
	m_painterSettings.m_color = color;
	m_painterSettings.m_colorHash[name] = color;
	
	Painter::onPaintColorsFinish();
	return;
}

void Painter::onChangePenSize(qint32 size)
{
	qDebug() << "Painter::onChangePenSize:" << size;
	m_painterSettings.m_penSize = size;
}

void Painter::setOpacity(qreal scaleOpacity)
{
	Logger->trace("Painter::setOpacity() set scale to:{}", scaleOpacity);
	m_paintPixmap->setOpacity(scaleOpacity);
}

void Painter::setOpacityROI(qreal scaleOpacity)
{
	Logger->trace("Painter::setOpacityROI() set scale to:{}", scaleOpacity);
	m_pixmap->setOpacity(scaleOpacity);
}

void Painter::setOpacityImage(qreal scaleOpacity)
{
	Logger->trace("Painter::setOpacityImage() set scale to:{}", scaleOpacity);
	m_pixmap->setOpacity(scaleOpacity);
}
