m_fontMetricsText = QFontMetrics(font);
// QRect rect = m_fontMetricsText.boundingRect(text);
// setFixedSize(rect.width(), rect.height() + 2);
int w = m_fontMetricsText.horizontalAdvance(text);
int h = m_fontMetricsText.height();


如果文本有中文符号时使用boundingRect计算的宽度不准确！应该使用horizontalAdvance