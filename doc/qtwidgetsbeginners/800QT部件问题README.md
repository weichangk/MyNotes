# QT部件问题

### QPushButton
QPushButton 在不设置固定宽度的时候能根据文本自适应宽度，但是文本长度不一样时左右间距不一样，需要设置 padding 来固定左右间距才能达到根据文本宽度自适应部件宽度的效果。


### 打开隐藏窗体部件样式没有还原默认状态
Awidget里的Bwidget的btn点击隐藏Awidget，重新打开显示Awidget发现btn的样式还是press的样式！


### 四态按钮背景色如果有透明度，在切换状态的时候颜色会有误差，透明度的问题！

### QSizePolicy类是一个描述布局水平和垂直方向调整策略的属性。
大小策略会影响布局引擎处理部件的方式，部件加入布局以后，会返回一个QSizePolicy，描述了其水平和垂直方向的大小策略。可以通过QWidget::sizePolicy属性为特定部件设置大小策略。

在水平或垂直布局中添加部件时，部件会自动垂直放大或水平放大，但是在添加按钮到布局中时往往不想设置按钮固定大小，又不想因为布局导致按钮自动放大时，可以设置按钮的QSizePolicy属性为，如下：
```
QSizePolicy sizePolicy = this->sizePolicy();
sizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
this->setSizePolicy(sizePolicy);
```

这样，按钮就可以根据按钮文本自适应大小，只需要在qss中设置padding即可


### QLabel设置的QPixmap自动缩放为QLabel大小
setScaledContents(true);

### QLineEdit 使用问题
```
QIntValidator *cropWEditValidator = new QIntValidator(this);
cropWEditValidator->setRange(kCropRectMinW, 1920);
crop_ratio_width_edit_->setValidator(cropWEditValidator);

// 使用 textEdited 事件实时判断
connect(crop_ratio_width_edit_, &ALineEdit::textEdited, this, [=](const QString text){
    if(text.toInt() < kCropRectMinW) {
        crop_ratio_width_edit_->setText(QString::number(kCropRectMinW));
    }
});
// 使用 editingFinished 事件确认最终值
connect(crop_ratio_width_edit_, &ALineEdit::editingFinished, this, [=](){
    // 不在 Range 范围内失去焦点或entrt都不会触发，returnPressed 按回车事件也不会触发
    qDebug() << "editingFinished:" << crop_ratio_width_edit_->text();
});
```