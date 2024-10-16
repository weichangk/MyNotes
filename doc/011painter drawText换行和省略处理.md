换行
```
QTextOption toption(Qt::AlignLeft);
toption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
painter->drawText(rcText, data.name, toption);
```

省略
```
QString elidedText = painter->fontMetrics().elidedText(name, Qt::ElideRight, text);
painter->drawText(rect, Qt::AlignLeft | Qt::AlignTop, elidedText);

```