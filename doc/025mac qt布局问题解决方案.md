```
auto setUserRect = [](QWidget *widget){
    auto childs = widget->children();
    for(auto child:childs){
    if(auto widget = dynamic_cast<QWidget*>(child)){
        widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    }
}};
setUserRect(m_pWidget);
```

Qt::WA_LayoutUsesWidgetRect 属性，用于强制子部件在布局中使用其实际的矩形区域，而不是默认的外边框。这个方式通常在mac下qt布局出现异常时有效