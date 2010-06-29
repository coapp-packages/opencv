//IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING. 

// By downloading, copying, installing or using the software you agree to this license.
// If you do not agree to this license, do not download, install,
// copy or use the software.


//                          License Agreement
//               For Open Source Computer Vision Library

//Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
//Copyright (C) 2008-2010, Willow Garage Inc., all rights reserved.
//Third party copyrights are property of their respective owners.

//Redistribution and use in source and binary forms, with or without modification,
//are permitted provided that the following conditions are met:

//  * Redistribution's of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.

//  * Redistribution's in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.

//  * The name of the copyright holders may not be used to endorse or promote products
//  derived from this software without specific prior written permission.

//This software is provided by the copyright holders and contributors "as is" and
//any express or implied warranties, including, but not limited to, the implied
//warranties of merchantability and fitness for a particular purpose are disclaimed.
//In no event shall the Intel Corporation or contributors be liable for any direct,
//indirect, incidental, special, exemplary, or consequential damages
//(including, but not limited to, procurement of substitute goods or services;
//loss of use, data, or profits; or business interruption) however caused
//and on any theory of liability, whether in contract, strict liability,
//or tort (including negligence or otherwise) arising in any way out of
//the use of this software, even if advised of the possibility of such damage.

//--------------------Google Code 2010 -- Yannick Verdie--------------------//


#ifdef HAVE_QT

#include <window_QT.h>
#include <QVarLengthArray>

//Static and global first
static GuiReceiver guiMainThread;
static int parameterSystemC = 1;
static char* parameterSystemV[] = {""};
static bool multiThreads = false;
static int last_key = -1;
QWaitCondition key_pressed;
QMutex mutexKey;

static const unsigned int threshold_zoom_img_region = 15;//the minimum zoom value to start displaying the values' grid
//end static and global

double cvGetRatioWindow_QT(const char* name)
{
	double result = -1;
    QMetaObject::invokeMethod(&guiMainThread,
                              "getRatioWindow",
                              //Qt::DirectConnection,
                              Qt::AutoConnection,
                              Q_RETURN_ARG(double, result),
                              Q_ARG(QString, QString(name)));
    return result;
}

void cvSetRatioWindow_QT(const char* name,double prop_value)
{
    QMetaObject::invokeMethod(&guiMainThread,
						  "setRatioWindow",
						  Qt::AutoConnection,
						  Q_ARG(QString, QString(name)),
                          Q_ARG(double, prop_value));
}

double cvGetPropWindow_QT(const char* name)
{
	double result = -1;
    QMetaObject::invokeMethod(&guiMainThread,
                              "getPropWindow",
                              //Qt::DirectConnection,
                              Qt::AutoConnection,
                              Q_RETURN_ARG(double, result),
                              Q_ARG(QString, QString(name)));
    return result;
}

void cvSetPropWindow_QT(const char* name,double prop_value)
{
    QMetaObject::invokeMethod(&guiMainThread,
						  "setPropWindow",
						  Qt::AutoConnection,
						  Q_ARG(QString, QString(name)),
                          Q_ARG(double, prop_value));
}

void cvSetModeWindow_QT(const char* name, double prop_value)
{
    QMetaObject::invokeMethod(&guiMainThread,
                              "toggleFullScreen",
                              Qt::AutoConnection,
                              Q_ARG(QString, QString(name)),
                              Q_ARG(double, prop_value));
}

double cvGetModeWindow_QT(const char* name)
{
	double result = -1;

    QMetaObject::invokeMethod(&guiMainThread,
                              "isFullScreen",
                              Qt::AutoConnection,
                              Q_RETURN_ARG(double, result),
                              Q_ARG(QString, QString(name)));
    return result;
}

CV_IMPL void cvDisplayOverlay(const char* name, const char* text, int delayms)
{

    QMetaObject::invokeMethod(&guiMainThread,
                              "displayInfo",
                              Qt::AutoConnection,
                              //Qt::DirectConnection,
                              Q_ARG(QString, QString(name)),
                              Q_ARG(QString, QString(text)),
                              Q_ARG(int, delayms));
                         
}

CV_IMPL void cvDisplayStatusBar(const char* name, const char* text, int delayms)
{

    QMetaObject::invokeMethod(&guiMainThread,
                              "displayStatusBar",
                              Qt::AutoConnection,
                              //Qt::DirectConnection,
                              Q_ARG(QString, QString(name)),
                              Q_ARG(QString, QString(text)),
                              Q_ARG(int, delayms));
}


CV_IMPL int cvInitSystem( int, char** )
{
	return 0;
}

CV_IMPL int cvWaitKey( int arg )
{
    int result = -1;

    unsigned long delayms;//in milliseconds
    if (arg<=0)
        delayms = ULONG_MAX;
    else
        delayms = arg;

    if (multiThreads)
    {
        mutexKey.lock();
        if(key_pressed.wait(&mutexKey,delayms))//false if timeout
        {
            result = last_key;
        }
        last_key = -1;
        mutexKey.unlock();

    }else{
        //cannot use wait here because events will not be distributed before processEvents (the main eventLoop is broken)
        //so I create a Thread for the QTimer

        QTimer timer(&guiMainThread);
        QObject::connect(&timer, SIGNAL(timeout()), &guiMainThread, SLOT(timeOut()));
        timer.setSingleShot(true);

        if (arg>0)
            timer.start(arg);

        while(!guiMainThread._bTimeOut)
        {
            qApp->processEvents(QEventLoop::AllEvents);

            mutexKey.lock();
            if (last_key != -1)
            {
                result = last_key;
                last_key = -1;
                timer.stop();
                //printf("keypressed\n");
            }
            mutexKey.unlock();

            if (result!=-1)
                break;
            else
                usleep(2);//to decrease CPU usage
        }
        guiMainThread._bTimeOut = false;
    }

    return result;
}

//Yannick Verdie
//This function is experimental and some functions (such as cvSet/getWindowProperty will not work)
//We recommend not using this function for now
CV_IMPL int cvStartLoop(int (*pt2Func)(int argc, char *argv[]), int argc, char* argv[])
{
    multiThreads = true;
    QFuture<int> future = QtConcurrent::run(pt2Func,argc,argv);
    return guiMainThread.start();
}

CV_IMPL void cvStopLoop()
{
    qApp->exit();
}


CV_IMPL CvWindow* icvFindWindowByName( const char* arg )
{

    QPointer<CvWindow> window = NULL;
    
    if( !arg )
        CV_Error( CV_StsNullPtr, "NULL name string" );

    QString name(arg);
    QPointer<CvWindow> w;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        w = (CvWindow*) widget;
        if (w->name==name)
        {
            window = w;
            break;
        }
    }
    
    return window;
}

CvTrackbar* icvFindTrackbarByName( const char* name_trackbar, const char* name_window )
{

    QPointer<CvTrackbar> result = NULL;
    
    QPointer<CvWindow> w = icvFindWindowByName( name_window );

    if( !w )
        CV_Error( CV_StsNullPtr, "NULL window handler" );

    QString nameQt = QString(name_trackbar);
    QPointer<CvTrackbar> t;

	//Warning   ----  , asume the location 0 is myview and max-1 the status bar
    for (int i = 1; i < w->layout->layout()->count()-2; ++i)
    {

        t = (CvTrackbar*) w->layout->layout()->itemAt(i);
        if (t->trackbar_name==nameQt)
        {
            result = t;
            break;
        }
    }

    return result;
}

CV_IMPL int icvInitSystem()
{
    static int wasInitialized = 0;

    // check initialization status
    if( !wasInitialized)
    {
		new QApplication(parameterSystemC,parameterSystemV);

        wasInitialized = 1;
        qDebug()<<"init done"<<endl;
    }

    return 0;
}

CV_IMPL int cvNamedWindow( const char* name, int flags )
{
    
    if (multiThreads)
        QMetaObject::invokeMethod(&guiMainThread,
                                  "createWindow",
                                  //Qt::AutoConnection,
                                  Qt::BlockingQueuedConnection,
                                  //TypeConnection,
                                  //Qt::AutoConnection,
                                  Q_ARG(QString, QString(name)),
                                  Q_ARG(int, flags));
    else
        guiMainThread.createWindow(QString(name),flags);

    return 1;//Dummy value
}

CV_IMPL void cvDestroyWindow( const char* name )
{
    
    QMetaObject::invokeMethod(&guiMainThread,
                              "destroyWindow",
                              //Qt::BlockingQueuedConnection,
                              Qt::AutoConnection,
                              Q_ARG(QString, QString(name))
                              );
}


CV_IMPL void cvDestroyAllWindows(void)
{

    QMetaObject::invokeMethod(&guiMainThread,
                              "destroyAllWindow",
                              //Qt::BlockingQueuedConnection,
                              Qt::AutoConnection
                              );

}

CV_IMPL void* cvGetWindowHandle( const char* name )
{
    if( !name )
        CV_Error( CV_StsNullPtr, "NULL name string" );

    return (void*) icvFindWindowByName( name );
}

CV_IMPL const char* cvGetWindowName( void* window_handle )
{
    
    if( !window_handle )
        CV_Error( CV_StsNullPtr, "NULL window handler" );

    return ((CvWindow*)window_handle)->windowTitle().toLatin1().data();
}

CV_IMPL void cvMoveWindow( const char* name, int x, int y )
{   

    
    QMetaObject::invokeMethod(&guiMainThread,
                              "moveWindow",
                              //Qt::BlockingQueuedConnection,
                              Qt::AutoConnection,
                              Q_ARG(QString, QString(name)),
                              Q_ARG(int, x),
                              Q_ARG(int, y)
                              );

}

CV_IMPL void cvResizeWindow(const char* name, int width, int height )
{

    QMetaObject::invokeMethod(&guiMainThread,
                              "resizeWindow",
                              //Qt::BlockingQueuedConnection,
                              Qt::AutoConnection,
                              Q_ARG(QString, QString(name)),
                              Q_ARG(int, width),
                              Q_ARG(int, height)
                              );

}

CV_IMPL int cvCreateTrackbar2( const char* trackbar_name, const char* window_name, int* val, int count, CvTrackbarCallback2 on_notify, void* userdata )
{
    //TODO: implement the real one, not a wrapper
    return cvCreateTrackbar( trackbar_name, window_name, val, count, (CvTrackbarCallback)on_notify );
}

CV_IMPL int cvStartWindowThread()
{
    return 0;
}

CV_IMPL int cvCreateTrackbar( const char* trackbar_name, const char* window_name, int* value, int count, CvTrackbarCallback on_change)
{
    
    if (multiThreads)
        QMetaObject::invokeMethod(&guiMainThread,
                                  "addSlider",
                                  Qt::AutoConnection,
                                  Q_ARG(QString, QString(trackbar_name)),
                                  Q_ARG(QString, QString(window_name)),
                                  Q_ARG(void*, (void*)value),
                                  Q_ARG(int, count),
                                  Q_ARG(void*, (void*)on_change)
                                  );
    else
        guiMainThread.addSlider(QString(trackbar_name),QString(window_name),(void*)value,count,(void*)on_change);

    return 1;//demmy value
}

CV_IMPL int cvGetTrackbarPos( const char* trackbar_name, const char* window_name )
{
    int result = -1;
   
    QPointer<CvTrackbar> t = icvFindTrackbarByName(  trackbar_name, window_name );

    if (t)
        result = t->slider->value();

    return result;
}

CV_IMPL void cvSetTrackbarPos( const char* trackbar_name, const char* window_name, int pos )
{
    
    QPointer<CvTrackbar> t = icvFindTrackbarByName(  trackbar_name, window_name );

    if (t)
        t->slider->setValue(pos);

}

/* assign callback for mouse events */
CV_IMPL void cvSetMouseCallback( const char* window_name, CvMouseCallback on_mouse,void* param )
{    
    QPointer<CvWindow> w = icvFindWindowByName( window_name );

    if (!w)
        CV_Error(CV_StsNullPtr, "NULL window handler" );

    w->setMouseCallBack(on_mouse, param);

}

CV_IMPL void cvShowImage( const char* name, const CvArr* arr )
{

	QMetaObject::invokeMethod(&guiMainThread,
							  "showImage",
							  //Qt::BlockingQueuedConnection,
							  Qt::DirectConnection,
							  Q_ARG(QString, QString(name)),
							  Q_ARG(void*, (void*)arr)
							  );
}








//----------OBJECT----------------

GuiReceiver::GuiReceiver() : _bTimeOut(false)
{
    icvInitSystem();
    qApp->setQuitOnLastWindowClosed ( false );//maybe the user would like to access this setting
}

double GuiReceiver::getRatioWindow(QString name)
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );


    if (!w)
        return -1;

    return (double)w->getView()->getRatio();
}

void GuiReceiver::setRatioWindow(QString name, double arg2 )
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (!w)
        return;

    int flags = (int) arg2;

    if (w->getView()->getRatio() == flags)//nothing to do
        return;

	//if valid flags
	if (flags == CV_WINDOW_FREERATIO || flags == CV_WINDOW_KEEPRATIO)
		w->getView()->setRatio(flags);

}

double GuiReceiver::getPropWindow(QString name)
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );


    if (!w)
        return -1;

    return (double)w->flags;
}

void GuiReceiver::setPropWindow(QString name, double arg2 )
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (!w)
        return;

    int flags = (int) arg2;

    if (w->flags == flags)//nothing to do
        return;


    switch(flags)
    {
    case  CV_WINDOW_NORMAL:
        w->layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        w->flags = flags;
        break;
    case  CV_WINDOW_AUTOSIZE:
        w->layout->setSizeConstraint(QLayout::SetFixedSize);
        w->flags = flags;
        break;
    default:;
    }
}

double GuiReceiver::isFullScreen(QString name)
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (!w)
        return -1;

    if (w->isFullScreen())
        return CV_WINDOW_FULLSCREEN;
    else
        return CV_WINDOW_NORMAL;
}

//accept CV_WINDOW_NORMAL or CV_WINDOW_FULLSCREEN
void GuiReceiver::toggleFullScreen(QString name, double flags )
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (!w)
        return;

    if (w->isFullScreen() && flags == CV_WINDOW_NORMAL)
        w->showNormal();

    if (!w->isFullScreen() && flags == CV_WINDOW_FULLSCREEN)
        w->showFullScreen();

}

void GuiReceiver::createWindow( QString name, int flags )
{
    if (!qApp)
        CV_Error(CV_StsNullPtr, "NULL session handler" );

    // Check the name in the storage
    if( icvFindWindowByName( name.toLatin1().data() ))
    {
        return;
    }

    //QPointer<CvWindow> w1 =
    new CvWindow(name, flags);
}

void GuiReceiver::refreshEvents()
{
    QAbstractEventDispatcher::instance(qApp->instance()->thread())->processEvents(QEventLoop::AllEvents);
    //qDebug()<<"refresh ?"<<endl;
    //qApp->processEvents(QEventLoop::AllEvents);
}

void GuiReceiver::timeOut()
{
    _bTimeOut = true;
}

void GuiReceiver::displayInfo( QString name, QString text, int delayms )
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (w && delayms > 0)
        w->displayInfo(text,delayms);
}

void GuiReceiver::displayStatusBar( QString name, QString text, int delayms )
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (w && delayms > 0)
        w->displayStatusBar(text,delayms);
}

void GuiReceiver::showImage(QString name, void* arr)
{
    //qDebug()<<"inshowimage"<<endl;
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (!w)//as observed in the previous implementation (W32, GTK or Carbon), create a new window is the pointer returned is null
    {
        cvNamedWindow( name.toLatin1().data() );
        w = icvFindWindowByName( name.toLatin1().data() );
    }

    if( w && arr )
    {
        w->updateImage(arr);
    }
    else
    {
        qDebug()<<"Do nothing (Window or Image NULL)"<<endl;
    }
}

void GuiReceiver::destroyWindow(QString name)
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (w)
    {
        w->close();
        //in not-multiThreads mode, looks like the window is hidden but not deleted
        //so I do it manually
        //otherwise QApplication do it for me if the exec command was executed (in multiThread mode)
        if (!multiThreads)
            delete w;
    }
}

void GuiReceiver::destroyAllWindow()
{
    if (!qApp)
        CV_Error(CV_StsNullPtr, "NULL session handler" );

    if (multiThreads)
    {
        qApp->closeAllWindows();
    }else{
        QPointer<CvWindow> w;
        foreach (QWidget *widget, QApplication::topLevelWidgets())
        {
            w = (CvWindow*) widget;
            w->close();
            delete w;
        }
    }

}

void GuiReceiver::moveWindow(QString name, int x, int y)
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (w)
        w->move(x,y);

}

void GuiReceiver::resizeWindow(QString name, int width, int height)
{
    QPointer<CvWindow> w = icvFindWindowByName( name.toLatin1().data() );

    if (w)
        w->resize(width, height);
}

void GuiReceiver::addSlider(QString trackbar_name, QString window_name, void* value, int count, void* on_change)
{
    QPointer<CvWindow> w = icvFindWindowByName( window_name.toLatin1().data()  );

    if (!w)
        return;

    if (!value)
        CV_Error(CV_StsNullPtr, "NULL value pointer" );

    if (count<= 0)//count is the max value of the slider, so must be bigger than 0
        CV_Error(CV_StsNullPtr, "Max value of the slider must be bigger than 0" );

    w->addSlider(trackbar_name,(int*)value,count,(CvTrackbarCallback) on_change);
}

int GuiReceiver::start()
{
    return qApp->exec();
}

CvTrackbar::CvTrackbar(CvWindow* arg, QString name, int* value, int count, CvTrackbarCallback on_change )
{
    setObjectName(trackbar_name);
    parent = arg;
    trackbar_name = name;
    dataSlider = value;

    callback = on_change;
    slider = new QSlider(Qt::Horizontal);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setMinimum(0);
    slider->setMaximum(count);
    slider->setPageStep(5);
    slider->setValue(*value);
    slider->setTickPosition(QSlider::TicksBelow);


    //Change style of the Slider
    slider->setStyleSheet(str_Trackbar_css);

    //QFile qss(PATH_QSLIDERCSS);
    //if (qss.open(QFile::ReadOnly))
    //{
    //    slider->setStyleSheet(QLatin1String(qss.readAll()));
    //    qss.close();
    //}


    //this next line does not work if we change the style with a stylesheet, why ? (bug in QT ?)
    //slider->setTickPosition(QSlider::TicksBelow);
    label = new QPushButton;
    label->setFlat(true);
    setLabel(slider->value());


    QObject::connect( slider, SIGNAL( valueChanged( int ) ),this, SLOT( update( int ) ) );

    QObject::connect( label, SIGNAL( clicked() ),this, SLOT( createDialog() ));

    //label->setStyleSheet("QPushButton:disabled {color: black}");

    addWidget(label);//name + value
    addWidget(slider);//slider
}

void CvTrackbar::createDialog()
{

    bool ok= false;

    //crash if I access the value directly to give them to QInputDialog, so do a copy first.
    int value = slider->value();
    int step = slider->singleStep();
    int min = slider->minimum();
    int max = slider->maximum();

    int i = QInputDialog::getInt(this->parentWidget(),
                                 tr("Slider %1").arg(trackbar_name),
                                 tr("New value:"),
                                 value,
                                 min,
                                 max,
                                 step,
                                 &ok);

    if (ok)
        slider->setValue(i);

}

void CvTrackbar::update(int myvalue)
{
    setLabel(myvalue);

    *dataSlider = myvalue;
    if (callback)
        callback(myvalue);
}

void CvTrackbar::setLabel(int myvalue)
{
    QString nameNormalized = trackbar_name.leftJustified( 10, ' ', true );
    QString valueMaximum = QString("%1").arg(slider->maximum());
    QString str = QString("%1 (%2/%3)").arg(nameNormalized).arg(myvalue,valueMaximum.length(),10,QChar('0')).arg(valueMaximum);
    label->setText(str);
}

CvTrackbar::~CvTrackbar()
{
    delete slider;
    delete label;
}

CvWindow::CvWindow(QString arg, int arg2)
{
    moveToThread(qApp->instance()->thread());
    name = arg;
    flags = arg2;

    setAttribute(Qt::WA_DeleteOnClose);//in other case, does not release memory
    setContentsMargins(0,0,0,0);
    setWindowTitle(name);
    setObjectName(name);

    resize(400,300);

    //CV_MODE_NORMAL or CV_MODE_OPENGL
    myview = new ViewPort(this, CV_MODE_NORMAL,CV_WINDOW_KEEPRATIO);//parent, mode_display, keep_aspect_ratio
    myview->setAlignment(Qt::AlignHCenter);


    shortcut_r_Zoom = new QShortcut(Qt::CTRL + Qt::Key_P, this);
    QObject::connect( shortcut_r_Zoom, SIGNAL( activated ()),myview, SLOT( resetZoom( ) ));
    shortcut_imgRegion = new QShortcut(Qt::CTRL + Qt::Key_O, this);
    QObject::connect( shortcut_imgRegion, SIGNAL( activated ()),myview, SLOT( imgRegion( ) ));
    shortcut_Plus = new QShortcut(QKeySequence(QKeySequence::ZoomIn), this);
    QObject::connect( shortcut_Plus, SIGNAL( activated ()),myview, SLOT( ZoomIn() ));
    shortcut_Minus = new QShortcut(QKeySequence(QKeySequence::ZoomOut), this);
    QObject::connect(shortcut_Minus, SIGNAL( activated ()),myview, SLOT( ZoomOut() ));
    shortcut_Left = new QShortcut(Qt::CTRL + Qt::Key_Left, this);
    QObject::connect( shortcut_Left, SIGNAL( activated ()),myview, SLOT( siftWindowOnLeft() ));
    shortcut_Right = new QShortcut(Qt::CTRL + Qt::Key_Right, this);
    QObject::connect( shortcut_Right, SIGNAL( activated ()),myview, SLOT( siftWindowOnRight() ));
    shortcut_Up = new QShortcut(Qt::CTRL + Qt::Key_Up, this);
    QObject::connect(shortcut_Up, SIGNAL( activated ()),myview, SLOT( siftWindowOnUp() ));
    shortcut_Down = new QShortcut(Qt::CTRL + Qt::Key_Down, this);
    QObject::connect(shortcut_Down, SIGNAL( activated ()),myview, SLOT( siftWindowOnDown() ));

    layout = new QBoxLayout(QBoxLayout::TopToBottom);
    //layout = new CustomLayout;
    layout->setObjectName(QString::fromUtf8("boxLayout"));
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(myview,Qt::AlignCenter);
    //layout->addStretch(0);

    if (flags == CV_WINDOW_AUTOSIZE)
        layout->setSizeConstraint(QLayout::SetFixedSize);

    //now status bar
    myBar = new QStatusBar;
    myBar->setSizeGripEnabled(false);
    myBar->setMaximumHeight(20);
    myBar_msg = new QLabel;
    myBar_msg->setFrameStyle(QFrame::Raised);
    myBar_msg->setAlignment(Qt::AlignHCenter);
    //myBar_msg->setWordWrap(true);
    myBar->addWidget(myBar_msg);
    layout->addWidget(myBar,Qt::AlignCenter);


    setLayout(layout);
    show();
}



CvWindow::~CvWindow()
{
    QLayoutItem *child;

    if (layout)
    {
        while ((child = layout->takeAt(0)) != 0)
            delete child;

        delete layout;
    }

    delete myBar;
    delete myBar_msg;


    delete shortcut_r_Zoom;
    delete shortcut_imgRegion;
    delete shortcut_Plus;
    delete shortcut_Minus;
    delete shortcut_Left;
    delete shortcut_Right;
    delete shortcut_Up;
    delete shortcut_Down;
}

ViewPort* CvWindow::getView()
{
	return myview;
}

void CvWindow::displayInfo(QString text,int delayms)
{
    myview->startDisplayInfo(text, delayms);
}

void CvWindow::displayStatusBar(QString text,int delayms)
{
    myBar->showMessage(text,delayms);
}

void CvWindow::updateImage(void* arr)
{
    myview->updateImage(arr);
}

void CvWindow::setMouseCallBack(CvMouseCallback m, void* param)
{
    myview->setMouseCallBack(m,param);
}

void CvWindow::addSlider(QString name, int* value, int count,CvTrackbarCallback on_change)
{
    QPointer<CvTrackbar> t = new CvTrackbar(this,name,value, count, on_change);
    t->setAlignment(Qt::AlignHCenter);
    layout->insertLayout(layout->count()-1,t);//max-1 means add trackbar between myview and statusbar
}

//Need more test here !
void CvWindow::keyPressEvent(QKeyEvent *event)
{
    //see http://doc.trolltech.com/4.6/qt.html#Key-enum
    int key = event->key();
    bool goodKey = false;

    if (key>=20 && key<=255 )
    {
        key = (int)event->text().toLocal8Bit().at(0);
        goodKey = true;
    }

    if (key == Qt::Key_Escape)
    {
        key = 27;
        goodKey = true;
    }

    //control plus (Z, +, -, up, down, left, right) are used for zoom/panning functions
    if (event->modifiers() != Qt::ControlModifier && goodKey)
    {
        mutexKey.lock();
        last_key = key;
        //last_key = event->nativeVirtualKey ();
        mutexKey.unlock();
        key_pressed.wakeAll();
        //event->accept();
    }

    QWidget::keyPressEvent(event);
}


void CvWindow::readSettings()//not tested
{
    QSettings settings("Trolltech", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void CvWindow::writeSettings()//not tested
{
    QSettings settings("Trolltech", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

//Here is ViewPort
ViewPort::ViewPort(CvWindow* arg, int arg2, int arg3)
{
    centralWidget = arg,
    mode = arg2;
    keepRatio = arg3;
    
    setupViewport(centralWidget);
    setContentsMargins(0,0,0,0);
    
    setObjectName(QString::fromUtf8("graphicsView"));
    timerDisplay = new QTimer(this);
    timerDisplay->setSingleShot(true);
    connect(timerDisplay, SIGNAL(timeout()), this, SLOT(stopDisplayInfo()));
    drawInfo = false;
    positionGrabbing = QPointF(0,0);
    positionCorners = QRect(0,0,size().width(),size().height());
    on_mouse = NULL;
    mouseCoordinate = QPoint(-1,-1);
    
    
#if defined(OPENCV_GL)
    if (mode == CV_MODE_OPENGL)
    {
	setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	initGL();
    }
#endif
    
    image2Draw_ipl=cvCreateImage(cvSize(centralWidget->width(),centralWidget->height()),IPL_DEPTH_8U,3);
    nbChannelOriginImage = 0;
    cvZero(image2Draw_ipl);
    
    setInteractive(false);
    setMouseTracking (true);//receive mouse event everytime
}

ViewPort::~ViewPort()
{
    if (image2Draw_ipl)
	cvReleaseImage(&image2Draw_ipl);
    
    delete timerDisplay;
}

void ViewPort::setRatio(int flags)
{
    keepRatio = flags;
    updateGeometry();
    viewport()->update();
}

int ViewPort::getRatio()
{
    return keepRatio;
}

void ViewPort::resetZoom()
{
    matrixWorld.reset();
    controlImagePosition();
}

void ViewPort::imgRegion( )
{
	scaleView(threshold_zoom_img_region/matrixWorld.m11(), QPointF(size().width()/2,size().height()/2),false);//false = do not process the 1st param 
}

void ViewPort::ZoomIn()
{
    scaleView( 0.5,QPointF(size().width()/2,size().height()/2),true);
}

void ViewPort::ZoomOut()
{
    scaleView( -0.5,QPointF(size().width()/2,size().height()/2),true);
}

//Note: move 2 percent of the window
void  ViewPort::siftWindowOnLeft()
{
    float delta = 2*width()/(100.0*matrixWorld.m11());
    moveView(QPointF(delta,0));
}

//Note: move 2 percent of the window
void  ViewPort::siftWindowOnRight()
{
    float delta = -2*width()/(100.0*matrixWorld.m11());
    moveView(QPointF(delta,0));
}

//Note: move 2 percent of the window
void  ViewPort::siftWindowOnUp()
{
    float delta = 2*height()/(100.0*matrixWorld.m11());
    moveView(QPointF(0,delta));
}

//Note: move 2 percent of the window
void  ViewPort::siftWindowOnDown()
{
    float delta = -2*height()/(100.0*matrixWorld.m11());
    moveView(QPointF(0,delta));
}

void ViewPort::startDisplayInfo(QString text, int delayms)
{
    if (timerDisplay->isActive())
	stopDisplayInfo();
    
    infoText = text;
    timerDisplay->start(delayms);
    drawInfo = true;
}

void ViewPort::stopDisplayInfo()
{
    timerDisplay->stop();
    drawInfo = false;
}

inline bool ViewPort::isSameSize(IplImage* img1,IplImage* img2)
{
    return img1->width == img2->width && img1->height == img2->height;
}

void ViewPort::updateImage(void* arr)
{
    if (!arr)
	CV_Error(CV_StsNullPtr, "NULL arr pointer (in showImage)" );
    
    IplImage* tempImage = (IplImage*)arr;
    
    if (!isSameSize(image2Draw_ipl,tempImage))
    {
	cvReleaseImage(&image2Draw_ipl);
	image2Draw_ipl=cvCreateImage(cvGetSize(tempImage),IPL_DEPTH_8U,3);
	
	nbChannelOriginImage = tempImage->nChannels;
	updateGeometry();
    }
    
    cvConvertImage(tempImage,image2Draw_ipl,CV_CVTIMG_SWAP_RB );
    
    viewport()->update();
}

void ViewPort::setMouseCallBack(CvMouseCallback m, void* param)
{
    on_mouse = m;
    on_mouse_param = param;
}

void ViewPort::controlImagePosition()
{
    qreal left, top, right, bottom;
    
    //after check top-left, bottom right corner to avoid getting "out" during zoom/panning
    matrixWorld.map(0,0,&left,&top);
    
    if (left > 0)
    {
	matrixWorld.translate(-left,0);
	left = 0;
    }
    if (top > 0)
    {
	matrixWorld.translate(0,-top);
	top = 0;
    }
    //-------
    
    QSize sizeImage = size();
    matrixWorld.map(sizeImage.width(),sizeImage.height(),&right,&bottom);
    if (right < sizeImage.width())
    {
	matrixWorld.translate(sizeImage.width()-right,0);
	right = sizeImage.width();
    }
    if (bottom < sizeImage.height())
    {
	matrixWorld.translate(0,sizeImage.height()-bottom);
	bottom = sizeImage.height();
    }
    
    //save corner position
    positionCorners.setTopLeft(QPoint(left,top));
    positionCorners.setBottomRight(QPoint(right,bottom));
    //save also the inv matrix
    matrixWorld_inv = matrixWorld.inverted();
    
    viewport()->update();
}

void ViewPort::moveView(QPointF delta)
{
    matrixWorld.translate(delta.x(),delta.y());
    controlImagePosition();
}

//factor is -0.5 (zoom out) or 0.5 (zoom in)
void ViewPort::scaleView(qreal factor,QPointF center,bool process_factor)
{
	if (process_factor)
	{
		factor/=5;//-0.1 <-> 0.1
		factor+=1;//0.9 <-> 1.1
	}
    
    //limit zoom out ---
    if (matrixWorld.m11()==1 && factor < 1)
	return;
    
    if (matrixWorld.m11()*factor<1)
	factor = 1/matrixWorld.m11();
    
    
    //limit zoom int ---
    if (matrixWorld.m11()>100 && factor > 1)
	return;
    
    //inverse the transform
    int a, b;
    matrixWorld_inv.map(center.x(),center.y(),&a,&b);
    
    matrixWorld.translate(a-factor*a,b-factor*b);
    matrixWorld.scale(factor,factor);
    
    controlImagePosition();
    
    //display new zoom
    centralWidget->displayStatusBar(tr("Zoom: %1%").arg(matrixWorld.m11()*100),1000);
    
    if (matrixWorld.m11()>1)
	setCursor(Qt::OpenHandCursor);
    else
	unsetCursor();
}

void ViewPort::wheelEvent(QWheelEvent *event)
{
    scaleView( -event->delta() / 240.0,event->pos(),true);//true means process the 1st parameter
}

void ViewPort::mousePressEvent(QMouseEvent *event)
{
    int cv_event = -1, flags = 0;
    QPoint pt = event->pos();
    
    //icvmouseHandler: pass parameters for cv_event, flags
    icvmouseHandler(event, mouse_down, cv_event, flags);
    icvmouseProcessing(QPointF(pt), cv_event, flags);
    
    if (matrixWorld.m11()>1)
    {
	setCursor(Qt::ClosedHandCursor);
	positionGrabbing = event->pos();
    }
    
    QWidget::mousePressEvent(event);
}

void ViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    
    int cv_event = -1, flags = 0;
    QPoint pt = event->pos();
    
    //icvmouseHandler: pass parameters for cv_event, flags
    icvmouseHandler(event, mouse_up, cv_event, flags);
    icvmouseProcessing(QPointF(pt), cv_event, flags);
    
    if (matrixWorld.m11()>1)
	setCursor(Qt::OpenHandCursor);
    
    QWidget::mouseReleaseEvent(event);
}

void ViewPort::mouseDoubleClickEvent(QMouseEvent *event)
{
    int cv_event = -1, flags = 0;
    QPoint pt = event->pos();
    
    //icvmouseHandler: pass parameters for cv_event, flags
    icvmouseHandler(event, mouse_dbclick, cv_event, flags);
    icvmouseProcessing(QPointF(pt), cv_event, flags);
    
    QWidget::mouseDoubleClickEvent(event);
}

void ViewPort::mouseMoveEvent(QMouseEvent *event)
{
    int cv_event = -1, flags = 0;
    QPoint pt = event->pos();
    
    //icvmouseHandler: pass parameters for cv_event, flags
    icvmouseHandler(event, mouse_move, cv_event, flags);
    icvmouseProcessing(QPointF(pt), cv_event, flags);
    
    
    if (matrixWorld.m11()>1 && event->buttons() == Qt::LeftButton)
    {
	QPointF dxy = (pt - positionGrabbing)/matrixWorld.m11();
	
	positionGrabbing = event->pos();
	
	moveView(dxy);
    }
    
    //I update the statusbar here because if the user does a cvWaitkey(0) (like with inpaint.cpp)
    //the status bar will only be repaint when a click occurs.
    viewport()->update();
    
    QWidget::mouseMoveEvent(event);
}

//up, down, dclick, move
void ViewPort::icvmouseHandler(QMouseEvent *event, type_mouse_event category, int &cv_event, int &flags)
{
    
    switch(event->modifiers())
    {
    case Qt::ShiftModifier:
	flags = CV_EVENT_FLAG_SHIFTKEY;
	break;
    case Qt::ControlModifier:
	flags = CV_EVENT_FLAG_CTRLKEY;
	break;
    case Qt::AltModifier:
	flags = CV_EVENT_FLAG_ALTKEY;
	break;
    case Qt::NoModifier	:
	break;
    case Qt::MetaModifier:
	break;
    case Qt::KeypadModifier:
	break;
    default:;
    }
    
    switch(event->button())
    {
    case Qt::LeftButton:
	cv_event = tableMouseButtons[category][0];
	flags |= CV_EVENT_FLAG_LBUTTON;
	break;
    case Qt::RightButton:
	cv_event = tableMouseButtons[category][1];
	flags |= CV_EVENT_FLAG_RBUTTON;
	break;
    case Qt::MidButton:
	cv_event = tableMouseButtons[category][2];
	flags |= CV_EVENT_FLAG_MBUTTON;
	break;
    default:;
    }
}

void ViewPort::icvmouseProcessing(QPointF pt, int cv_event, int flags)
{
    //to convert mouse coordinate
    qreal pfx, pfy;
    matrixWorld_inv.map(pt.x(),pt.y(),&pfx,&pfy);
    mouseCoordinate.rx()=floor(pfx);
    mouseCoordinate.ry()=floor(pfy);
    
    if (on_mouse)
	on_mouse( cv_event, mouseCoordinate.x(),mouseCoordinate.y(), flags, on_mouse_param );
    
}

QSize ViewPort::sizeHint() const
{
    if(image2Draw_ipl)
    {
	return QSize(image2Draw_ipl->width,image2Draw_ipl->height);
    } else {
	return QGraphicsView::sizeHint();
    }
}

void ViewPort::resizeEvent ( QResizeEvent *event)
{
    
    controlImagePosition();
    //ratioX=float(image2Draw_ipl->width)/float(width());
    //ratioY=float(image2Draw_ipl->height)/float(height());
    ratioX=width()/float(image2Draw_ipl->width);
    ratioY=height()/float(image2Draw_ipl->height);
    
    if(keepRatio == CV_WINDOW_KEEPRATIO)//to keep the same aspect ratio
    {
	QSize newSize = QSize(image2Draw_ipl->width,image2Draw_ipl->height);
	newSize.scale(event->size(),Qt::KeepAspectRatio);
	
	//imageWidth/imageHeight = newWidth/newHeight +/- epsilon
	//ratioX = ratioY +/- epsilon
	//||ratioX - ratioY|| = epsilon
	if (fabs(ratioX - ratioY)*100> ratioX)//avoid infinity loop / epsilon = 1% of ratioX
	{
	    resize(newSize);
	    
	    //move to the middle
	    //newSize get the delta offset to place the picture in the middle of its parent
	    newSize= (event->size()-newSize)/2;
	    move(newSize.width(),newSize.height());
	}
    }
    
    return QGraphicsView::resizeEvent(event);
}

void ViewPort::paintEvent(QPaintEvent* event)
{
    QPainter myPainter(viewport());
    myPainter.setWorldTransform(matrixWorld);
    
    draw2D(&myPainter);
    
    
#if defined(OPENCV_GL)
    if (mode == CV_MODE_OPENGL && false)//disable it for now
    {
	setGL(this->width(),this->height());
	draw3D();
	unsetGL();
    }
#endif
    
    //in mode zoom/panning
    if (matrixWorld.m11()>1)
    {
	
	myPainter.setWorldMatrixEnabled (false );
	
	if (matrixWorld.m11()>=threshold_zoom_img_region)
	    drawImgRegion(&myPainter);
	
	drawViewOverview(&myPainter);
	
    }
    
    //for statusbar
    drawStatusBar();
    
    //for information overlay
    if (drawInfo)
    {
	myPainter.setWorldMatrixEnabled (false );
	drawInstructions(&myPainter);
    }
    
    QGraphicsView::paintEvent(event);
}

void ViewPort::draw2D(QPainter *painter)
{
    image2Draw_qt = QImage((uchar*) image2Draw_ipl->imageData, image2Draw_ipl->width, image2Draw_ipl->height,QImage::Format_RGB888);
    // painter->drawImage(0,0,image2Draw_qt.scaled(this->width(),this->height(),Qt::KeepAspectRatio));
    painter->drawImage(0,0,image2Draw_qt.scaled(this->width(),this->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    
}



void ViewPort::drawStatusBar()
{
    if (mouseCoordinate.x()>=0 &&
	mouseCoordinate.y()>=0 &&
	mouseCoordinate.x()<image2Draw_ipl->width &&
	mouseCoordinate.y()<image2Draw_ipl->height)
    {
	QRgb rgbValue = image2Draw_qt.pixel(mouseCoordinate);
	
	if (nbChannelOriginImage==3)
	{
	    centralWidget->myBar_msg->setText(tr("<font color='black'>Coordinate: %1x%2 ~ </font>")
					      .arg(mouseCoordinate.x())
					      .arg(mouseCoordinate.y())+
					      tr("<font color='red'>R:%3 </font>").arg(qRed(rgbValue))+//.arg(value.val[0])+
					      tr("<font color='green'>G:%4 </font>").arg(qGreen(rgbValue))+//.arg(value.val[1])+
					      tr("<font color='blue'>B:%5</font>").arg(qBlue(rgbValue))//.arg(value.val[2])
					      );
	}else{
	    //all the channel have the same value (because of cvconvertimage), so only the r channel is dsplayed
	    centralWidget->myBar_msg->setText(tr("<font color='black'>Coordinate: %1x%2 ~ </font>")
					      .arg(mouseCoordinate.x())
					      .arg(mouseCoordinate.y())+
					      tr("<font color='grey'>grey:%3 </font>").arg(qRed(rgbValue))
					      );
	}
    }
}

void ViewPort::drawImgRegion(QPainter *painter)
{
    qreal offsetX = matrixWorld.dx()/matrixWorld.m11();
    offsetX = offsetX - floor(offsetX);
    qreal offsetY = matrixWorld.dy()/matrixWorld.m11();
    offsetY = offsetY - floor(offsetY);
    
    QSize view = size();
    QVarLengthArray<QLineF, 30> linesX;
    for (qreal x = offsetX*matrixWorld.m11(); x < view.width(); x += matrixWorld.m11() )
    	linesX.append(QLineF(x, 0, x, view.height()));
    
    QVarLengthArray<QLineF, 30> linesY;
    for (qreal y = offsetY*matrixWorld.m11(); y < view.height(); y += matrixWorld.m11() )
	linesY.append(QLineF(0, y, view.width(), y));
    
    
    QFont f = painter->font();
    f.setPointSize(threshold_zoom_img_region/3+(matrixWorld.m11()-threshold_zoom_img_region)/6);
    //f.setStretch(0);
    painter->setFont(f);
    QString val;
    QRgb rgbValue;
    
    QPointF point1;//sorry, I do not know how to name it
    QPointF point2;//idem
    
    
    for (int j=-1;j<view.height()/matrixWorld.m11();j++)
	for (int i=-1;i<view.width()/matrixWorld.m11();i++)
	{
	point1.setX((i+offsetX)*matrixWorld.m11());
	point1.setY((j+offsetY)*matrixWorld.m11());
	
	matrixWorld_inv.map(point1.x(),point1.y(),&point2.rx(),&point2.ry());
	
	if (point2.x() >= 0 && point2.y() >= 0)
	    rgbValue = image2Draw_qt.pixel(QPoint(point2.x(),point2.y()));
	else
	    rgbValue = qRgb(0,0,0);
	
	const int margin = 1; 
	if (nbChannelOriginImage==3)
	{
	    val = tr("%1").arg(qRed(rgbValue));
	    painter->setPen(QPen(Qt::red, 1));
	    painter->drawText(QRect(point1.x(),point1.y()+margin,matrixWorld.m11(),matrixWorld.m11()/3),
			      Qt::AlignCenter, val);
	    
	    val = tr("%1").arg(qGreen(rgbValue));
	    painter->setPen(QPen(Qt::green, 1));
	    painter->drawText(QRect(point1.x(),point1.y()+matrixWorld.m11()/3+margin,matrixWorld.m11(),matrixWorld.m11()/3),
			      Qt::AlignCenter, val);
	    
	    val = tr("%1").arg(qBlue(rgbValue));
	    painter->setPen(QPen(Qt::blue, 1));
	    painter->drawText(QRect(point1.x(),point1.y()+2*matrixWorld.m11()/3+margin,matrixWorld.m11(),matrixWorld.m11()/3),
			      Qt::AlignCenter, val);
	    
	}
	else
	{
	    
	    val = tr("%1").arg(qRed(rgbValue));
	    painter->drawText(QRect(point1.x(),point1.y(),matrixWorld.m11(),matrixWorld.m11()),
			      Qt::AlignCenter, val);
	}
    }
    
    painter->setPen(QPen(Qt::black, 1));
    painter->drawLines(linesX.data(), linesX.size());
    painter->drawLines(linesY.data(), linesY.size());
    
}

void ViewPort::drawViewOverview(QPainter *painter)
{
    QSize viewSize = size();
    viewSize.scale ( 100, 100,Qt::KeepAspectRatio );
    
    const int margin = 5;
    
    //draw the image's location
    painter->setBrush(QColor(0, 0, 0, 127));
    painter->setPen(Qt::darkGreen);
    painter->drawRect(QRect(width()-viewSize.width()-margin, 0,viewSize.width(),viewSize.height()));
    
    //daw the view's location inside the image
    qreal ratioSize = 1/matrixWorld.m11();
    qreal ratioWindow = (qreal)(viewSize.height())/(qreal)(size().height());
    painter->setPen(Qt::darkBlue);
    painter->drawRect(QRectF(width()-viewSize.width()-positionCorners.left()*ratioSize*ratioWindow-margin,
			     -positionCorners.top()*ratioSize*ratioWindow,
			     (viewSize.width()-1)*ratioSize,
			     (viewSize.height()-1)*ratioSize)
		      );
}

void ViewPort::drawInstructions(QPainter *painter)
{
    QFontMetrics metrics = QFontMetrics(font());
    int border = qMax(4, metrics.leading());
    
    QRect rect = metrics.boundingRect(0, 0, width() - 2*border, int(height()*0.125),
				      Qt::AlignCenter | Qt::TextWordWrap, infoText);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->fillRect(QRect(0, 0, width(), rect.height() + 2*border),
		      QColor(0, 0, 0, 127));
    painter->setPen(Qt::white);
    painter->fillRect(QRect(0, 0, width(), rect.height() + 2*border),
		      QColor(0, 0, 0, 127));
    painter->drawText((width() - rect.width())/2, border,
		      rect.width(), rect.height(),
		      Qt::AlignCenter | Qt::TextWordWrap, infoText);
}














#if defined(OPENCV_GL)//all this section -> not tested

void ViewPort::initGL()
{
    glShadeModel( GL_SMOOTH );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
}

//from http://steinsoft.net/index.php?site=Programming/Code%20Snippets/OpenGL/gluperspective
//do not want to link glu
void ViewPort::icvgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;
    
    ymax = zNear * tan(fovy * M_PI / 360.0);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;
    
    
    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}


void ViewPort::setGL(int width, int height)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    icvgluPerspective(45, float(width) / float(height), 0.01, 1000);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void ViewPort::unsetGL()
{
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void ViewPort::draw3D()
{
    //draw scene here
    glLoadIdentity();
    
    glTranslated(0.0, 0.0, -1.0);
    // QVector3D p = convert(positionMouse);
    //glTranslated(p.x(),p.y(),p.z());
    
    glRotatef( 55, 1, 0, 0 );
    glRotatef( 45, 0, 1, 0 );
    glRotatef( 0, 0, 0, 1 );
    
    static const int coords[6][4][3] = {
	{ { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
	{ { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
	{ { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
	{ { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
	{ { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
	{ { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };
    
    for (int i = 0; i < 6; ++i) {
	glColor3ub( i*20, 100+i*10, i*42 );
	glBegin(GL_QUADS);
	for (int j = 0; j < 4; ++j) {
	    glVertex3d(0.2 * coords[i][j][0], 0.2 * coords[i][j][1], 0.2 * coords[i][j][2]);
	}
	glEnd();
    }
}
#endif

#endif
