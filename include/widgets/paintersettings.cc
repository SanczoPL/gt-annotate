#include "paintersettings.h"

PainterSettings::PainterSettings()
{

}

void PainterSettings::configureColors(QJsonObject const& a_config)
{
    Logger->trace("PainterSettings::configureColors()");
    auto _colors = a_config[COLORS].toArray();
    for (int i = 0; i < _colors.size(); i++)
    {
        auto _colorsIter = _colors[i].toObject();
        QString _name = _colorsIter[NAME].toString();
        m_colors.push_back(_name);
        int _r = _colorsIter[R].toInt();
        int _g = _colorsIter[G].toInt();
        int _b = _colorsIter[B].toInt();
        int _a = _colorsIter[A].toInt();
        int _gray = _colorsIter[GRAY_COLOR].toInt();

        m_colorInthash.insert(_name, _gray);
        m_colorHash.insert(_name, { _r, _g, _b, _a });
    }
    qDebug() << "m_colorInthash:" << m_colorInthash;
    qDebug() << "m_colorHash:" << m_colorHash;
}
