#include "gameview.h"
#include "ui_gameview.h"


#include <QtGui>
#include <QtCore>
#include <QMessageBox>

class MainWindow;
#include "mainwindow.h"
extern MainWindow *w;

GameView::GameView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameView)
{
    ui->setupUi(this);
    qsrand(QTime::currentTime().msec());

    totalTileAmount = 0;
    boardEdgeSize = w->data->getBoardEdgeSizeValue();

    timeLeft = w->data->getTimeLimitValue();
    if(w->data->isTimeLimitChecked() && timeLeft<=20)
    {
            w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusicHurry,34500);
    }
    else
        w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusic,76800);

    isBackgroundMusicOn = true;
    isGamePaused = false;

    keyEventBlock = false;

    //Initialize the control panel properties
    controlPanelWidth = 120;
    controlPanelHeight = 400;

    //Initialize the basic properties
    // boardEdgeSize = w->data->getBoardEdgeSizeValue();
    gap = 5;
    tileEdgeLength = 80;
    gameAreaEdgeLength = (gap+tileEdgeLength)*(w->data->getBoardEdgeSizeValue())+gap;
    goal = w->data->getGoalValue();
    setFixedSize(gap + controlPanelWidth + ((gap+tileEdgeLength) * (w->data->getBoardEdgeSizeValue())+gap),
                       2 * gap + ((gap+tileEdgeLength)*(w->data->getBoardEdgeSizeValue())+gap));
    ui->label_scoreText->setGeometry(0,0,controlPanelWidth,30);
    ui->label_scoreValue->setGeometry(0,30,controlPanelWidth,30);
    ui->label_goalText->setGeometry(0,70,controlPanelWidth,30);
    ui->label_goalValue->setGeometry(0,100,controlPanelWidth,30);
    ui->label_timeText->setGeometry(0,140,controlPanelWidth,30);
    ui->label_timeValue->setGeometry(0,170,controlPanelWidth,30);


    ui->pushButton_restartTheGame->setGeometry(-30,
                                             -30,
                                             0,
                                             0);
    ui->pushButton_IDontWantToPlay->setGeometry(8,
                                             gap+(w->data->getBoardEdgeSizeValue()-2)*(tileEdgeLength+gap)+tileEdgeLength/2+45,
                                             controlPanelWidth-8-8,
                                             50);
    ui->pushButton_goBackToMenu->setGeometry(8,
                                             gap+(w->data->getBoardEdgeSizeValue()-1)*(tileEdgeLength+gap)+tileEdgeLength/2+15,
                                             controlPanelWidth-8-8,
                                             30);
    ui->pushButton_quiet->setGeometry(10,
                                      gap+(w->data->getBoardEdgeSizeValue()-2)*(tileEdgeLength+gap)+tileEdgeLength/2,
                                      40,
                                      40);
    ui->pushButton_pause->setGeometry(70,
                                      gap+(w->data->getBoardEdgeSizeValue()-2)*(tileEdgeLength+gap)+tileEdgeLength/2,
                                      40,
                                      40);
    setTileColor(w->data->getTileColor());

    score = 0;
    ui->label_scoreValue->setText(QString::number(score));
    ui->label_scoreValue->setStyleSheet("QLabel{background-color : transparent ; color : blue}");
    ui->label_scoreValue->setAlignment(Qt::AlignCenter);


    ui->label_goalValue->setText(QString::number(w->data->getGoalValue()));
    ui->label_goalValue->setStyleSheet("QLabel{background-color : transparent ; color : red}");
    ui->label_goalValue->setAlignment(Qt::AlignCenter);

    //Set stopwatch properties
    timerForStopWatch = new QTimer;
    if(w->data->isTimeLimitChecked())
    {
        setStopWatchValueAndShow(timeLeft);

        counterForStopWatch = 0;
        connect(timerForStopWatch,SIGNAL(timeout()),this,SLOT(oneTimeUnitPass()));
        timerForStopWatch->start(100);
    }
    else{
        ui->label_timeValue->setText("N/A");
    }
    ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : green}");
    ui->label_timeValue->setAlignment(Qt::AlignCenter);


    //**Set the properties for animation START

    isThisIndexGenerateNewTile = new bool[power(boardEdgeSize,2)];
    timerForMove = new QTimer;
    timerForEmerge = new QTimer;

    //**Set the properties for animation END



    //gameArea origin : (controlPanelWidth,gap)
    gameAreaPosX = controlPanelWidth;
    gameAreaPosY = gap;
    rectBlock = new QGraphicsRectItem[power(w->data->getBoardEdgeSizeValue(),2)];

    //initialize tile, label_demoValue

    tile = new QGraphicsRectItem*[power(boardEdgeSize,2)];
    for(int i=0;i<power(boardEdgeSize,2);i++)
    {
        (*(tile+i)) = NULL;   //prevent from deleteing twice
    }

    label_demoValue = new QLabel*[power(boardEdgeSize,2)];
    for(int i=0;i<power(boardEdgeSize,2);i++)
        (*(label_demoValue+i)) = NULL;  //prevent from deleteing twice

    currentValueOfTile = new int[power(boardEdgeSize,2)];
    for(int i=0;i<power(boardEdgeSize,2);i++)   //initialize currentValueOfTile
        *(currentValueOfTile+i) = 0; //value 0 means empty

    nextValueOfTile = new int[power(boardEdgeSize,2)];
    for(int i=0;i<power(boardEdgeSize,2);i++)   //initialize nextValueOfTile
        *(nextValueOfTile+i) = 0;

    nextPosOfTile = new int[power(boardEdgeSize,2)];
    for(int i=0;i<power(boardEdgeSize,2);i++)
        *(nextPosOfTile+i) = -1; //initialize nextPosOfTile;

    //create a scene and set its properties
    gameAreaScene = new QGraphicsScene;
    gameAreaScene->setSceneRect(0,0,(gap+tileEdgeLength)*(w->data->getBoardEdgeSizeValue())+gap-2,
                        (gap+tileEdgeLength)*(w->data->getBoardEdgeSizeValue())+gap-2);
    //set graphicsView properties and set scene to view
    ui->graphicsView->setGeometry(gameAreaPosX,gameAreaPosY,gameAreaEdgeLength,gameAreaEdgeLength);
    ui->graphicsView->setScene(gameAreaScene);

    //Initialize gameStatusLabel
    gameStatusLabel = NULL; //would be allocated later

    //The object above would be added to scene later

    //Start to set rectBlock
    for(int row = 0;row<(w->data->getBoardEdgeSizeValue());row++)
        for(int col = 0;col<(w->data->getBoardEdgeSizeValue());col++)
        {
            //set each rectBlock's rect
            rectBlock[row+col*(w->data->getBoardEdgeSizeValue())] . setRect(0,0,tileEdgeLength,tileEdgeLength);
            rectBlock[row+col*(w->data->getBoardEdgeSizeValue())] . setPos(gap + col * (tileEdgeLength+gap),gap + row * (tileEdgeLength+gap));
            //add rectBlocks to the scene
            gameAreaScene->addItem(rectBlock+row+col*(w->data->getBoardEdgeSizeValue()));

          }
    //Basic game scene had been set up above,
    //Now it's about time to set the gameAreaFrame
    gameAreaFrame.setRect(0,0,
            (gap+tileEdgeLength)*(w->data->getBoardEdgeSizeValue())+gap-3,
            (gap+tileEdgeLength)*(w->data->getBoardEdgeSizeValue())+gap-3);
    gameAreaFrame.setPen(QPen(getTileColor(power(2,2)),3));
    gameAreaFrame.setBrush(Qt::transparent);
    //setFlag(QGraphicsItem::ItemIsFocusable);
    //setFocus();
    gameAreaScene->addItem(&gameAreaFrame);
    gameAreaFrame.setPos(0,0);




    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //qDebug()<<"<gameview.cpp>graphicsView show";
    setFocusPolicy(Qt::ClickFocus);
    generateTile();
}

GameView::~GameView()
{
    w->sound->soundBgMusicPlay_stop();
    delete ui;
    delete [] currentValueOfTile;
    delete [] nextValueOfTile;
    delete [] nextPosOfTile;
    delete [] rectBlock;
    delete gameStatusLabel;
    delete timerForStopWatch;
    delete timerForMove;
    delete timerForEmerge;
    //delete gameAreaScene;

}

void GameView::keyPressEvent(QKeyEvent *event)  //step 1:receive key event from player
{
    //會傳event的情況:  1.尚未gameEnd(gameOver or gameWin)
                    qDebug() << event->key()<<"Key detected";
    if(!keyEventBlock)    //key尚未被block，則傳event
    {

            if(event->key()==Qt::Key_Up||event->key()==Qt::Key_W)
            {
                emitMoveTileSignalToAll(QString("Up"));
            }
            else if(event->key()==Qt::Key_Down||event->key()==Qt::Key_S)
            {
                emitMoveTileSignalToAll(QString("Down"));
            }
            else if(event->key()==Qt::Key_Left||event->key()==Qt::Key_A)
            {
                emitMoveTileSignalToAll(QString("Left"));
            }
            else if(event->key()==Qt::Key_Right||event->key()==Qt::Key_D)
            {
                emitMoveTileSignalToAll(QString("Right"));
            }


            else if(event->key()==Qt::Key_P)    //pause/resume game
            {
                on_pushButton_pause_clicked();
            }
    }
    else    //keyEvent is blocked
    {
        if(isGamePaused)
        {
            if(event->key()==Qt::Key_P)
                on_pushButton_pause_clicked();

        }
    }
            qDebug() << "Step0:keyPressEvent done";
}
void GameView::emitMoveTileSignalToAll(QString motion)
{
    isAnyTileMoved = false;


    //copy currentValueOfTile to nextValueOfTile
    for(int i=0;i<power(boardEdgeSize,2);i++)
        *(nextValueOfTile+i) = *(currentValueOfTile+i);


    if(motion == QString("Up"))
    {
        qDebug() << "motion Up detected";
        //由上往下發指令
        for(int row=0;row<boardEdgeSize;++row)
            for(int col=0;col<boardEdgeSize;++col)
                moveTile(col+row*boardEdgeSize,motion);  //Would trigger function moveTile()
    }
    else if(motion == QString("Down"))
    {
        //由下往上發指令
        for(int row=boardEdgeSize-1;row>=0;--row)
            for(int col=0;col<boardEdgeSize;++col)
                moveTile(col+row*boardEdgeSize,motion);  //Would trigger function moveTile()
    }
    else if(motion == QString("Left"))
    {
        //由左往右發指令
        for(int col=0;col<boardEdgeSize;++col)
            for(int row=0;row<boardEdgeSize;++row)
                moveTile(col+row*boardEdgeSize,motion);  //Would trigger function moveTile()}
    }
    else if(motion == QString("Right"))
    {
        //由右往左發指令
        for(int col=boardEdgeSize-1;col>=0;--col)
            for(int row=0;row<boardEdgeSize;++row)
                moveTile(col+row*boardEdgeSize,motion);  //Would trigger function moveTile()
    }

    if(isAnyTileMoved)  //If any tile is moved, then generate tile;Otherwise do nothing
    {
        generateTile(); //step3
    }
            qDebug() << "Step1:emitMoveSignalToAll done";

}



void GameView::moveTile(int index,QString motion) //step 2:move tiles
//this function moves a tile only, so moving all the tiles need to call this function several times
{
    if((*(currentValueOfTile+index)) == 0)
        *(nextPosOfTile+index) = -1;   //-1 means not to move
    else
    {
        int row,col,currentRow,currentCol;
        int step = 0;
        if(motion == QString("Up"))
        {
                    qDebug() << "motion Up received";
            row = index/boardEdgeSize;
            col = index%boardEdgeSize;
            currentRow = row;
            currentCol = col;
            //由該tile往上檢查
            while(1)
            {
                row--;
                if(row<0)
                    break;
                if((*(nextValueOfTile+col+row*boardEdgeSize)) == 0)   //Available
                    step++;
                else
                {
                    if((*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) == (*(nextValueOfTile+col+row*boardEdgeSize)))
                    {
                        step++;
                        break;
                    }
                    else
                        break;
                }
            }
            if(step>0)
            {
                isAnyTileMoved = true;
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = currentCol+(currentRow-step)*boardEdgeSize;
                if(*(nextValueOfTile+currentCol+(currentRow-step)*boardEdgeSize) == 0)
                {
                    (*(nextValueOfTile+currentCol+(currentRow-step)*boardEdgeSize)) = (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    *(nextValueOfTile+currentCol+currentRow*boardEdgeSize) = 0;
                }
                else    // target!=0
                {
                    (*(nextValueOfTile+currentCol+(currentRow-step)*boardEdgeSize)) += (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    *(nextValueOfTile+currentCol+currentRow*boardEdgeSize) = 0;

                    scoreAddAndShow(*(nextValueOfTile+currentCol+(currentRow-step)*boardEdgeSize));
                }
            }
            else    //step == 0
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = -1;

        }
        else if(motion == QString("Down"))
        {
            row = index/boardEdgeSize;
            col = index%boardEdgeSize;
            currentRow = row;
            currentCol = col;
            //由該tile往下檢查
            while(1)
            {
                row++;
                if(row>=boardEdgeSize)
                    break;
                if((*(nextValueOfTile+col+row*boardEdgeSize)) == 0)   //Available
                    step++;
                else
                {
                    if((*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) == (*(nextValueOfTile+col+row*boardEdgeSize)))
                    {
                        step++;
                        break;
                    }
                    else
                        break;
                }
            }
            if(step>0)
            {
                isAnyTileMoved = true;
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = currentCol+(currentRow+step)*boardEdgeSize;
                if((*(nextValueOfTile+currentCol+(currentRow+step)*boardEdgeSize)) == 0)
                {
                    (*(nextValueOfTile+currentCol+(currentRow+step)*boardEdgeSize)) = (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    *(nextValueOfTile+currentCol+currentRow*boardEdgeSize) = 0;
                }
                else    // target!=0
                {
                    (*(nextValueOfTile+currentCol+(currentRow+step)*boardEdgeSize)) += (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    *(nextValueOfTile+currentCol+currentRow*boardEdgeSize) = 0;
                    scoreAddAndShow(*(nextValueOfTile+currentCol+(currentRow+step)*boardEdgeSize));
                }
            }
            else    //step == 0
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = -1;

        }
        else if(motion == QString("Left"))
        {
            row = index/boardEdgeSize;
            col = index%boardEdgeSize;
            currentRow = row;
            currentCol = col;
            //由該tile往左檢查

            while(1)
            {
                col--;
                if(col<0)
                    break;
                if((*(nextValueOfTile+col+row*boardEdgeSize)) == 0)   //Available
                    step++;
                else
                {
                    if((*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) == (*(nextValueOfTile+col+row*boardEdgeSize)))
                    {
                        step++;
                        break;
                    }
                    else
                        break;
                }
            }
            if(step>0)
            {
                isAnyTileMoved = true;
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = (currentCol-step)+currentRow*boardEdgeSize;
                if((*(nextValueOfTile+(currentCol-step)+currentRow*boardEdgeSize)) == 0)
                {
                    (*(nextValueOfTile+(currentCol-step)+currentRow*boardEdgeSize)) = (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) = 0;
                }
                else    // target!=0
                {
                    (*(nextValueOfTile+(currentCol-step)+currentRow*boardEdgeSize)) += (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) = 0;
                    scoreAddAndShow((*(nextValueOfTile+(currentCol-step)+currentRow*boardEdgeSize)));
                }
            }
            else    //step == 0
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = -1;

        }
        else if(motion == QString("Right"))
        {
            row = index/boardEdgeSize;
            col = index%boardEdgeSize;
            currentRow = row;
            currentCol = col;
            //由該tile往右檢查
            while(1)
            {
                col++;
                if(col>=boardEdgeSize)
                    break;
                if((*(nextValueOfTile+col+row*boardEdgeSize)) == 0)   //Available
                    step++;
                else
                {
                    if((*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) == (*(nextValueOfTile+col+row*boardEdgeSize)))
                    {
                        step++;
                        break;
                    }
                    else
                        break;
                }
            }
            if(step>0)
            {
                isAnyTileMoved = true;
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = (currentCol+step)+currentRow*boardEdgeSize;
                if((*(nextValueOfTile+(currentCol+step)+currentRow*boardEdgeSize) == 0))
                {
                    (*(nextValueOfTile+(currentCol+step)+currentRow*boardEdgeSize)) = (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) = 0;
                }
                else    // target!=0
                {
                    (*(nextValueOfTile+(currentCol+step)+currentRow*boardEdgeSize)) += (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize));
                    (*(nextValueOfTile+currentCol+currentRow*boardEdgeSize)) = 0;
                    scoreAddAndShow(*(nextValueOfTile+(currentCol+step)+currentRow*boardEdgeSize));
                }
            }
            else    //step == 0;
                *(nextPosOfTile+currentCol+currentRow*boardEdgeSize) = -1;

        }
    }
        qDebug() << "Step2:moveTile done";
}

void GameView::generateTile()   //step 3
{
    //Initialize isThisIndexGenerateNewTile for step 4-2
    for(int i=0;i<power(boardEdgeSize,2);i++)
        *(isThisIndexGenerateNewTile+i) = false;
    int squareBoardEdgeSize = power(boardEdgeSize,2);
    int *ballot = new int[squareBoardEdgeSize];
    int current_index=0;

    //製籤
    for(int i=0;i<power(boardEdgeSize,2);i++)
    {
        if(*(nextValueOfTile+i)==0) //empty
        {
            //將空的tile加到ballot中，待會要抽
            ballot[current_index]=i;
            current_index++;

        }
    }
    //total_index_available_amount:可使用的籤的個數，即可配置的位置的個數
    int total_index_available_amount = current_index;
    //開始產生tile(開始抽籤以決定要產生的位置
    //先設定要產生幾個tile
    //從data讀取每一步要產生的*tile的個數
    int tileAmountToGenerate = (w->data->getTileGenerateAmount());
    for(int i=0;i<tileAmountToGenerate;i++)
    {
        if(total_index_available_amount <= 0)
        {
    //當可產生的位置數量=0時，跳出迴圈，不再生成新的*tile
            break;
        }
        current_index = qrand()%(total_index_available_amount);

        (*(nextValueOfTile+ballot[current_index])) = 2;  //設定初值 //step4 會實作tile的動畫
        *(isThisIndexGenerateNewTile+ballot[current_index]) = true; //將生成新tile的繪製記錄下來，待會step4做tile的emerge動畫時要用

        //籤已被用走一個->該籤需從ballot中刪掉
        for(int j=current_index;j<total_index_available_amount-1;j++)
            ballot[j] = ballot[j+1];

        //已配置一個新的*tile，so total_index_available_amount -1;
        total_index_available_amount--;
    }
    //qDebug()<<"currentValueOfTile:";
    /*for(int i=0;i<boardEdgeSize;i++)
        qDebug()<<*(currentValueOfTile+i*boardEdgeSize)<<" "<<*(currentValueOfTile+i*boardEdgeSize+1)
               <<" "<<*(currentValueOfTile+i*boardEdgeSize+2)<<" "<<*(currentValueOfTile+i*boardEdgeSize+3);*/

    qDebug() << "Step3:generateTile done";
            tileAniMove();    //step4

    // recycle dynamically allocated resources
    delete[] ballot;
}






void GameView::tileAniMove()    //step 4-1 tile動畫實作:Move
{
    keyEventBlock = true;   //Block keyPressEvent during the tile animation
    //keyEventBlock would be unlocked in the end of step 6
    counterForMoveStep = 0;
    connect(timerForMove,SIGNAL(timeout()),this,SLOT(tileAniMoveImplementation()));
    timerForMove->start(10);    //move a tiny step in 15 ms
}


void GameView::checkIfAnyTileReachGoal()    //step 5:檢查是否有tile的value已到達目標
{
    bool gameIsWin = false;
    for(int i=0;i<power(boardEdgeSize,2);i++)
    {
        if( *(currentValueOfTile+i) >= goal)
        {
            gameIsWin = true;
            break;

        }
    }
           qDebug() << "Step5:checkIfAnyTileReachGoal done";
    if(gameIsWin)
    {
                gameWin();  //如果已有tile的值已達到目標，則執行gameWin()
    }
    else
    {
               checkIfAnyTileIsMovable();	//如果還沒贏，則進入step6:檢查還有沒有可以動的*tile

    }

}

void GameView::checkIfAnyTileIsMovable()    //step 6:檢查還有沒有可以動的tile
{
    //檢查是否所有*tile皆已不能動
       bool allTileCanNotMove = true;
       //向右及向下檢查是否有1.空的tile 2.同樣value的tile存在
       //有的話就代表tile還可以移動，allTileCanNotMove設為false
       for(int row=0;row<boardEdgeSize;row++)
           for(int col=0;col<boardEdgeSize;col++)
           {
               //向左檢查
               if(col-1<0)	//如果左邊沒東西就不用檢查
                   ;//do nothing
               else
               {
                   //如果左邊是空的tile，就把allTileCanNotMove設為false
                   if(*(currentValueOfTile+(col-1)+row*boardEdgeSize)==0)  //Available
                       allTileCanNotMove =  false;

                   //如果左邊是和自己相同value的tile，就把allTileCanNotMove設為false
                   else if((*(currentValueOfTile+col+row*boardEdgeSize))==(*(currentValueOfTile+(col-1)+row*boardEdgeSize)))
                       allTileCanNotMove = false;
               }
               //向右檢查
               if(col+1>=boardEdgeSize)	//如果右邊沒東西就不用檢查
                   ;//do nothing
               else
               {
                   //如果右邊是空的tile，就把allTileCanNotMove設為false
                   if(*(currentValueOfTile+(col+1)+row*boardEdgeSize)==0)  //Available
                       allTileCanNotMove =  false;

                   //如果右邊是和自己相同value的tile，就把allTileCanNotMove設為false
                   else if((*(currentValueOfTile+col+row*boardEdgeSize))==(*(currentValueOfTile+(col+1)+row*boardEdgeSize)))
                       allTileCanNotMove = false;
               }
               //向上檢查
               if(row-1<0)	//如果上面沒東西就不用檢查
                   ;//do nothing
               else
               {
                   //如果上面是空的tile，就把allTileCanNotMove設為false
                   if(*(currentValueOfTile+col+(row-1)*boardEdgeSize)==0)  //Available
                       allTileCanNotMove = false;
                   //如果上面是和自己相同value的tile，就把allTileCanNotMove設為false
                   else if((*(currentValueOfTile+col+row*boardEdgeSize))==(*(currentValueOfTile+col+(row-1)*boardEdgeSize)))
                       allTileCanNotMove = false;
               }
               //向下檢查
               if(row+1>=boardEdgeSize)	//如果下面沒東西就不用檢查
                   ;//do nothing
               else
               {
                   //如果下面是空的tile，就把allTileCanNotMove設為false
                   if(*(currentValueOfTile+col+(row+1)*boardEdgeSize)==0)  //Available
                       allTileCanNotMove = false;
                   //如果下面是相同value的tile，就把allTileCanNotMove設為false
                   else if((*(currentValueOfTile+col+row*boardEdgeSize))==(*(currentValueOfTile+col+(row+1)*boardEdgeSize)))
                       allTileCanNotMove = false;
               }

           }

       if(allTileCanNotMove)
       {
           //如果所有*tile皆不能動，則gameOver
           gameOver();
       }
       //如果還有tile能動的話，則解除對keyPressEvent的block並退出此函式，等待下一次的keyPressEvent
       else
            keyEventBlock = false;

       /*qDebug()<<"nextPosOfTile";
       for(int i=0;i<boardEdgeSize;i++)
           qDebug()<<*(nextPosOfTile+i*boardEdgeSize)<<" "<<*(nextPosOfTile+i*boardEdgeSize+1)
                  <<" "<<*(nextPosOfTile+i*boardEdgeSize+2)<<" "<<*(nextPosOfTile+i*boardEdgeSize+3);
       qDebug()<<"nextValueOfTile";
       for(int i=0;i<boardEdgeSize;i++)
           qDebug()<<*(nextValueOfTile+i*boardEdgeSize)<<" "<<*(nextValueOfTile+i*boardEdgeSize+1)
                  <<" "<<*(nextValueOfTile+i*boardEdgeSize+2)<<" "<<*(nextValueOfTile+i*boardEdgeSize+3);*/

       qDebug() << "Step6:checkIfAnyTileMovable done";

}



void GameView::tileCreator(int index,int value)
{
    //1.建置label_demoValue's value、position
    //2.刷新tile's rect's position
    tileRectCreator(index,value);
    tileTextCreator(index,value);
}

void GameView::tileTextCreator(int index,int value)
{
    int row = index/boardEdgeSize;
    int col = index%boardEdgeSize;
    (*(label_demoValue+index)) = new QLabel;

    (*(label_demoValue+index))->setGeometry(gap+(gap+tileEdgeLength)*col,gap+(gap+tileEdgeLength)*row,tileEdgeLength,tileEdgeLength);
    (*(label_demoValue+index))->setText(QString::number(value));
    QFont font;
    if(value >= 1000)
        font.setPointSize(24);
    else if(value >= 100)
        font.setPointSize(28);
    else
        font.setPointSize(32);
    font.setBold(true);
    (*(label_demoValue+index))->setFont(font);
    if(tileColor==QString("white"))
        (*(label_demoValue+index))->setStyleSheet("QLabel{background-color : transparent ; color : black}");
    else
    {
        if(tileColor==QString("black"))
        {
            int k = 1;
            while(1)
            {
                if(power(2,k)==value)
                    break;
                k++;
            }
            if(k%5==0)
                (*(label_demoValue+index))->setStyleSheet("QLabel{background-color : transparent ; color : black}");
            else
                (*(label_demoValue+index))->setStyleSheet("QLabel{background-color : transparent ; color : white}");
        }
        else
            (*(label_demoValue+index))->setStyleSheet("QLabel{background-color : transparent ; color : white}");
    }
    (*(label_demoValue+index))->setAlignment(Qt::AlignCenter);
    gameAreaScene->addWidget((*(label_demoValue+index)));
}

void GameView::tileRectCreator(int index,int value)
{
    int row = index/boardEdgeSize;
    int col = index%boardEdgeSize;
    (*(tile+index)) = new QGraphicsRectItem;
    (*(tile+index))->setRect(0,0,tileEdgeLength,tileEdgeLength);
    (*(tile+index))->setPos(gap+(gap+tileEdgeLength)*col,gap+(gap+tileEdgeLength)*row);
    //依照tileColor來決定pen(外框)的顏色
    if(tileColor == QString("white"))
        (*(tile+index))->setPen(QPen(Qt::black,3));
    else
        (*(tile+index))->setPen(QPen(getTileColor(value),1));
    (*(tile+index))->setBrush(getTileColor(value));
    gameAreaScene->addItem((*(tile+index)));
}

void GameView::tileDestructor(int index)
{
    delete (*(tile+index));
    *(tile+index) = NULL;   //prevent from deleteing twice
    delete (*(label_demoValue+index));
    *(label_demoValue+index) = NULL;    //prevent from deleteing twice
}

void GameView::tmpDisplayEmergeTileInitializer(int selfIndex,int index, int value)
{
    tmpDisplayEmergeTileRectInitializer(selfIndex,index,value);
    //tmpDisplayEmergeTileTextInitializer(selfIndex,index,value);
}

void GameView::tmpDisplayEmergeTileRectInitializer(int selfIndex,int index, int value)
{
    int row = index/boardEdgeSize;
    int col = index%boardEdgeSize;
    tmpDisplayEmergeTileEdgeLength = 0;
    (tmpDisplayEmergeTile+selfIndex)->setRect(0,
                                              0,
                                              tmpDisplayEmergeTileEdgeLength,
                                              tmpDisplayEmergeTileEdgeLength);
    (tmpDisplayEmergeTile+selfIndex)->setPos(gap+(gap+tileEdgeLength)*col+tileEdgeLength/2-tmpDisplayEmergeTileEdgeLength/2,
                                             gap+(gap+tileEdgeLength)*row+tileEdgeLength/2-tmpDisplayEmergeTileEdgeLength/2);
    //依照tileColor來決定pen(外框)的顏色
    if(tileColor == QString("white"))
        (tmpDisplayEmergeTile+selfIndex)->setPen(QPen(Qt::black,3));
    else
        (tmpDisplayEmergeTile+selfIndex)->setPen(QPen(getTileColor(value),1));
    (tmpDisplayEmergeTile+selfIndex)->setBrush(getTileColor(value));
    gameAreaScene->addItem(tmpDisplayEmergeTile+selfIndex);
}

/*void GameView::tmpDisplayEmergeTileTextInitializer(int selfIndex,int index, int value)
{
    int row = index/boardEdgeSize;
    int col = index%boardEdgeSize;
    tmpDisplayEmergeTileEdgeLength = 12;
    (tmpDisplayEmergeLabel+selfIndex)->setGeometry(gap+(gap+tileEdgeLength)*col+tileEdgeLength/2-tmpDisplayEmergeTileEdgeLength/2,
                                                   gap+(gap+tileEdgeLength)*row+tileEdgeLength/2-tmpDisplayEmergeTileEdgeLength/2,
                                                   tmpDisplayEmergeTileEdgeLength,
                                                   tmpDisplayEmergeTileEdgeLength);
    QFont font;
    if(value >= 1000)
        tmpDisplayEmergeLabelFontSize = 7;
    else if(value >= 100)
        tmpDisplayEmergeLabelFontSize = 11;
    else
        tmpDisplayEmergeLabelFontSize = 15;
    font.setPointSize(tmpDisplayEmergeLabelFontSize);
    font.setBold(true);
    (tmpDisplayEmergeLabel+selfIndex)->setFont(font);
    if(tileColor==QString("white"))
        (tmpDisplayEmergeLabel+selfIndex)->setStyleSheet("QLabel{background-color : transparent ; color : black}");
    else
        (tmpDisplayEmergeLabel+selfIndex)->setStyleSheet("QLabel{background-color : transparent ; color : white}");
    (tmpDisplayEmergeLabel+selfIndex)->setAlignment(Qt::AlignCenter);
    gameAreaScene->addWidget(tmpDisplayEmergeLabel+selfIndex);
}*/

void GameView::tmpDisplayEmergeTileDestructor(int selfIndex)
{
    //delete (tmpDisplayEmergeLabel+selfIndex);
    delete (tmpDisplayEmergeTile+selfIndex);
}

void GameView::scoreAddAndShow(int variation)
{
    score += variation;
    ui->label_scoreValue->setText(QString::number(score));
}

void GameView::gameStatusLabelCreator()
{
    gameStatusLabel = new QLabel;
    gameStatusLabel->setGeometry(0,0,gameAreaEdgeLength,gameAreaEdgeLength);
    gameStatusLabel->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPointSize(28);
    font.setBold(true);
    gameStatusLabel->setFont(font);
}

void GameView::gameStatusLabelDestructor()
{
    delete gameStatusLabel;
    gameStatusLabel = NULL;
}

void GameView::setTileColor(QString color)
{
    tileColor = color;
}

QColor GameView::getTileColor(int tileValue)
{
    QColor color[5];    //5levels of tile
    if(tileColor == QString("white"))
    {
        return QColor(Qt::white);
    }
    else if(tileColor == QString("red"))
    {
        color[0] = QColor(0xff,0x88,0x88);    //index higher, color darker
        color[1] = QColor(0xff,0x33,0x33);
        color[2] = QColor(0xff,0x00,0x00);
        color[3] = QColor(0xcc,0x00,0x00);
        color[4] = QColor(0x88,0x00,0x00);
    }
    else if(tileColor == QString("yellow"))
    {
        color[0] = QColor(0xff,0xdd,0x55);    //index higher, color darker
        color[1] = QColor(0xff,0xbb,0x00);
        color[2] = QColor(0xff,0x88,0x00);
        color[3] = QColor(0xbb,0x55,0x00);
        color[4] = QColor(0x88,0x66,0x00);
    }
    else if(tileColor == QString("green"))
    {
        color[0] = QColor(0x99,0xff,0x99);    //index higher, color darker
        color[1] = QColor(0x99,0xff,0x33);
        color[2] = QColor(0x00,0xee,0x00);
        color[3] = QColor(0x00,0xbb,0x00);
        color[4] = QColor(0x00,0x88,0x00);
    }
    else if(tileColor == QString("blue"))
    {
        color[0] = QColor(0x00,0xff,0xff);    //index higher, color darker
        color[1] = QColor(0x00,0xbb,0xff);
        color[2] = QColor(0x00,0x66,0xff);
        color[3] = QColor(0x00,0x44,0xbb);
        color[4] = QColor(0x00,0x00,0x88);
    }
    else if(tileColor == QString("purple"))
    {
        color[0] = QColor(0xff,0xb3,0xff);    //index higher, color darker
        color[1] = QColor(0xff,0x3e,0xff);
        color[2] = QColor(0xcc,0x00,0xff);
        color[3] = QColor(0x77,0x00,0xbb);
        color[4] = QColor(0x3a,0x00,0x88);
    }
    else if(tileColor == QString("black"))
    {
        color[0] = QColor(0x00,0x00,0x00);    //index higher, color brighter
        color[1] = QColor(0x55,0x55,0x55);
        color[2] = QColor(0x99,0x99,0x99);
        color[3] = QColor(0xbb,0xbb,0xbb);
        color[4] = QColor(0xee,0xee,0xee);
    }
    int k=1;
    while(1)
    {
        if(power(2,k) == tileValue)
        {
            return color[(k-1)%5];
        }
        k++;
    }
}

void GameView::oneTimeUnitPass()
{
    counterForStopWatch++;
    if(counterForStopWatch >= 10)
    {
        timeLeft--;
        setStopWatchValueAndShow(timeLeft);
        counterForStopWatch = 0;

    }
    if(timeLeft == 20)
    {
        if(counterForStopWatch==0)
        {
            w->sound->soundBgMusicPlay_stop();
            w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusicHurry,34500);
        }
    }


    //The following are for text twinkling effect
    if(timeLeft <= 20)
    {
        if(timeLeft > 10)   //timeLeft:20~10
        {
                ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : orange}");

        }
        else if(timeLeft > 5)   //timeLeft:10~5
        {
            if(counterForStopWatch%5 == 0)
                ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : yellow}");
            else if(counterForStopWatch%5 == 2)
                ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : orange}");


        }
        else if(timeLeft > 0)   //timeLeft:5~0
        {
            if(counterForStopWatch%2 == 0)
                ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : red}");
            else if(counterForStopWatch%2 == 1)
                ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : yellow}");
        }
        else    //timeLeft:0
        {
            ui->label_timeValue->setStyleSheet("QLabel{background-color : transparent ; color : red}");
            gameOver();
        }
    }
}






void GameView::tileAniMoveImplementation()
{
    int moveUnit = 5;   //in pixel
    int index;
    //根據nextPosOfTile和currentValueOfTile做出tile動畫
    //在動畫完成後，根據nextValueOfTile建立新的tile
    //Start to implement move animation
    for(int row=0;row<boardEdgeSize;row++)
        for(int col=0;col<boardEdgeSize;col++)
        {
            index = col + row*boardEdgeSize;
            if((*(nextPosOfTile+index))==-1)    //-1 means not to move
                ;   //do nothing
            else if(*(tile+col+row*boardEdgeSize)==NULL)
                ;   //do nothing
            else
            {
                int posDifference = *(nextPosOfTile+index)-index;   //nextPos-currentPos
                int displacement;   //unit:block
                if(posDifference/boardEdgeSize < 0)    //move up
                {
                    displacement = posDifference/boardEdgeSize;
                    (*(tile+col+row*boardEdgeSize))->setPos((*(tile+col+row*boardEdgeSize))->x(),
                                                            (*(tile+col+row*boardEdgeSize))->y()+displacement*moveUnit);
                    (*(label_demoValue+col+row*boardEdgeSize))->setGeometry((*(label_demoValue+col+row*boardEdgeSize))->x(),
                                                                            (*(label_demoValue+col+row*boardEdgeSize))->y()+displacement*moveUnit,
                                                                            tileEdgeLength,
                                                                            tileEdgeLength);

                }
                else if(posDifference/boardEdgeSize > 0) //move down
                {
                    displacement = posDifference/boardEdgeSize;
                    (*(tile+col+row*boardEdgeSize))->setPos((*(tile+col+row*boardEdgeSize))->x(),
                                                            (*(tile+col+row*boardEdgeSize))->y()+displacement*moveUnit);
                    (*(label_demoValue+col+row*boardEdgeSize))->setGeometry((*(label_demoValue+col+row*boardEdgeSize))->x(),
                                                                            (*(label_demoValue+col+row*boardEdgeSize))->y()+displacement*moveUnit,
                                                                            tileEdgeLength,
                                                                            tileEdgeLength);

                }
                else if(posDifference/boardEdgeSize == 0)
                {
                    if(posDifference%boardEdgeSize < 0)  //move left
                    {
                        displacement = posDifference%boardEdgeSize;
                        (*(tile+col+row*boardEdgeSize))->setPos((*(tile+col+row*boardEdgeSize))->x()+displacement*moveUnit,
                                                                (*(tile+col+row*boardEdgeSize))->y());
                        (*(label_demoValue+col+row*boardEdgeSize))->setGeometry((*(label_demoValue+col+row*boardEdgeSize))->x()+displacement*moveUnit,
                                                                                (*(label_demoValue+col+row*boardEdgeSize))->y(),
                                                                                tileEdgeLength,
                                                                                tileEdgeLength);
                    }
                    else if(posDifference%boardEdgeSize > 0) //move right
                    {
                        displacement = posDifference%boardEdgeSize;
                        (*(tile+col+row*boardEdgeSize))->setPos((*(tile+col+row*boardEdgeSize))->x()+displacement*moveUnit,
                                                                (*(tile+col+row*boardEdgeSize))->y());
                        (*(label_demoValue+col+row*boardEdgeSize))->setGeometry((*(label_demoValue+col+row*boardEdgeSize))->x()+displacement*moveUnit,
                                                                                (*(label_demoValue+col+row*boardEdgeSize))->y(),
                                                                                tileEdgeLength,
                                                                                tileEdgeLength);
                    }
                }

            }

        }

    counterForMoveStep++;
    if(counterForMoveStep == 13)    //step 4-2:tile動畫實作:Move
    {
        //prepare emerge animation
        newTileAmount = 0;
        for(int i=0;i<power(boardEdgeSize,2);i++)
            if(*(isThisIndexGenerateNewTile+i))
                newTileAmount++;
        tmpDisplayEmergeTile = new QGraphicsRectItem[newTileAmount];
        int tmpTileIndex = 0;
        for(int index=0;index<power(boardEdgeSize,2);index++)   //To assign information (based on isThisIndexGenerateNewTile)to tmpDisplayEmergeTile
            if(*(isThisIndexGenerateNewTile+index))
            {
                tmpDisplayEmergeTileInitializer(tmpTileIndex,index,*(nextValueOfTile+index));
                tmpTileIndex++;
                qDebug()<<"tmpDisplayEmergeTile "<<tmpTileIndex<<" initialized";
            }
        qDebug()<<"newTileAmount:"<<newTileAmount;
        counterForEmergeStep = 0;
        connect(timerForEmerge,SIGNAL(timeout()),this,SLOT(tileAniEmergeImplementation()));
        timerForEmerge->start(15);
    }
    if(counterForMoveStep >= 17)    //finish the animation  //17(moveStepAmount)*15(each step cost time)=255
    {
        timerForMove->stop();
        disconnect(timerForMove,SIGNAL(timeout()),this,SLOT(tileAniMoveImplementation()));
        for(int i=0;i<power(boardEdgeSize,2);i++) //將畫面依照nextValueOfTile重新建置
        {
            if(*(nextValueOfTile+i)!=0)
            {
                if(*(isThisIndexGenerateNewTile+i))
                    tileDestructor(i);  //delete the pos where is to generate tile because of the emerge animation
                else
                {

                    tileDestructor(i);
                    tileCreator(i,*(nextValueOfTile+i));
                }
            }
            else    //empty tile
                tileDestructor(i);
        }
        qDebug() << "Step4-1:tileAniMove done";
    }

}


void GameView::tileAniEmergeImplementation()
{
    int scaleUnit = 5;  //in pixel
    //Start to implement emerge animation
    //QFont font;
    for(int i=0;i<newTileAmount;i++)
    {
        //rect emerge animation
        (tmpDisplayEmergeTile+i)->setRect(0,
                                          0,
                                          tmpDisplayEmergeTileEdgeLength + scaleUnit*2,
                                          tmpDisplayEmergeTileEdgeLength + scaleUnit*2);
        (tmpDisplayEmergeTile+i)->setPos((tmpDisplayEmergeTile+i)->x() - scaleUnit,
                                         (tmpDisplayEmergeTile+i)->y() - scaleUnit);
        //label emerge animation
        /*(tmpDisplayEmergeLabel+i)->setGeometry((tmpDisplayEmergeLabel+i)->x()-scaleUnit,
                                               (tmpDisplayEmergeLabel+i)->y()-scaleUnit,
                                               (tmpDisplayEmergeLabel+i)->width()+scaleUnit*2,
                                               (tmpDisplayEmergeLabel+i)->height()+scaleUnit*2);
        font.setPointSize(tmpDisplayEmergeLabelFontSize+1);
        (tmpDisplayEmergeLabel+i)->setFont(font);*/

    }
    tmpDisplayEmergeTileEdgeLength += scaleUnit*2;
    //tmpDisplayEmergeLabelFontSize++;

    counterForEmergeStep++;

    if(counterForEmergeStep >= 8)
    {
        timerForEmerge->stop();
        disconnect(timerForEmerge,SIGNAL(timeout()),this,SLOT(tileAniEmergeImplementation()));
        for(int i=0;i<newTileAmount;i++)    //delete tmpDisplayEmerge tile
            tmpDisplayEmergeTileDestructor(i);
        for(int i=0;i<power(boardEdgeSize,2);i++) //將step4-1尚未重新建置的tile(即new tile)重新建置
        {
            if(*(isThisIndexGenerateNewTile+i))
               tileCreator(i,*(nextValueOfTile+i));
        }

        //動畫實作完畢，可以將currentValueOfTile用nextValueOfTile覆蓋過去了
        for(int i=0;i<power(boardEdgeSize,2);i++)
            *(currentValueOfTile+i) = *(nextValueOfTile+i);
        qDebug() << "Step4-2:tileAniEmerge done";
        checkIfAnyTileReachGoal();  //進入step5
    }
}





void GameView::setStopWatchValueAndShow(int totalSecond)
{
    int min,sec;
    min = totalSecond/60;
    sec = totalSecond%60;
    if(sec < 10)
        ui->label_timeValue->setText(QString::number(min)+":0"+QString::number(sec));
    else
        ui->label_timeValue->setText(QString::number(min)+":"+QString::number(sec));
}



int GameView::getTileEdgeLengthValue()
{
    return tileEdgeLength;
}


int GameView::getGapValue()
{
    return gap;
}

int GameView::getCurrentValueOfTile(int index)
{
    return *(currentValueOfTile+index);
}

void GameView::setCurrentValueOfTile(int index, int value)
{
    *(currentValueOfTile+index) = value;
}


int GameView::power(int a, int n)
{
    int sum = 1;
    for(int i=0;i<n;i++)
        sum = sum * a;
    return sum;
}

void GameView::gameOver()
{
    w->sound->soundPlay_stop();
    w->sound->soundBgMusicPlay_stop();
    w->sound->soundPlay(w->sound->gameOverMusic);
    timerForStopWatch->stop();
    keyEventBlock = true;   //Block all the keyPressEvent when gameOver
    isGamePaused = false;
    if(gameStatusLabel == NULL)
        gameStatusLabelCreator();
    gameStatusLabel->setText("Game Over\n(oДo)\"");
    gameStatusLabel->setStyleSheet("QLabel{background-color : rgba(0,0,0,100) ; color : red}");

    gameAreaScene->addWidget(gameStatusLabel);
    ui->pushButton_restartTheGame->setGeometry(controlPanelWidth+gap+(gap+tileEdgeLength)*(boardEdgeSize/2.0-0.75),
                                               gap+(gap+tileEdgeLength)*(boardEdgeSize/2.0+0.8),
                                               gap+tileEdgeLength*1.5,
                                               40);
    ui->pushButton_restartTheGame->setStyleSheet("QPushButton{background-color : rgba(255,255,255,200)}");
    ui->pushButton_pause->setEnabled(false);
    ui->pushButton_quiet->setEnabled(false);
}

void GameView::gameWin()
{
    w->sound->soundBgMusicPlay_stop();
    w->sound->soundPlay(w->sound->gameWinMusic);
    timerForStopWatch->stop();
    keyEventBlock = true;   //Block all the keyPressEvent when gameWin
    isGamePaused = false;
    if(gameStatusLabel == NULL)
        gameStatusLabelCreator();
    gameStatusLabel->setText("You Win The Game\n\\^o^/");
    gameStatusLabel->setStyleSheet("QLabel{background-color : rgba(255,200,0,100) ; color : white}");
    gameAreaScene->addWidget(gameStatusLabel);
    ui->pushButton_restartTheGame->setGeometry(controlPanelWidth+gap+(gap+tileEdgeLength)*(boardEdgeSize/2.0-0.75),
                                               gap+(gap+tileEdgeLength)*(boardEdgeSize/2.0+0.8),
                                               gap+tileEdgeLength*1.5,
                                               40);
    ui->pushButton_restartTheGame->setStyleSheet("QPushButton{background-color : rgba(255,255,255,200)}");
    ui->pushButton_pause->setEnabled(false);
    ui->pushButton_quiet->setEnabled(false);
}


void GameView::on_pushButton_IDontWantToPlay_clicked()
{
    ui->pushButton_pause->setIcon(QIcon(":/images/resource/deadface.png"));
    ui->pushButton_quiet->setIcon(QIcon(":/images/resource/deadface.png"));
    ui->pushButton_pause->setEnabled(false);
    ui->pushButton_quiet->setEnabled(false);

    ui->pushButton_IDontWantToPlay->setIcon(QIcon(":/images/resource/deadface.png"));
    ui->pushButton_IDontWantToPlay->setIconSize(QSize(40,40));

    gameOver();
}

void GameView::on_pushButton_restartTheGame_clicked()
{
    w->sound->soundPlay_stop();
    w->on_pushButton_gameRestart_clicked();
}

void GameView::on_pushButton_goBackToMenu_clicked()
{
    QMessageBox::StandardButton reply;
    w->sound->soundPlay(w->sound->messageAlert);
    reply = QMessageBox::question(this,"Go back to Menu","Do you really want to go back to Menu?\n"
                                  "Current game status would be DISCARDED!!",
                          QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
        this->close();
}

void GameView::on_pushButton_quiet_clicked()
{
    if(isBackgroundMusicOn)
    {
        w->sound->soundBgMusicPlay_stop();
        ui->pushButton_quiet->setIcon(QIcon(":/images/resource/speakerQuiet.png"));
        ui->pushButton_quiet->setIconSize(QSize(30,30));
        isBackgroundMusicOn = false;
    }
    else    //background music is off
    {
        if(!isGamePaused)   //Resume music when game is in playing status
        {
            if(timeLeft<=20)
                w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusicHurry,34500);
            else
                w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusic,76800);
        }
        ui->pushButton_quiet->setIcon(QIcon(":/images/resource/speaker.png"));
        ui->pushButton_quiet->setIconSize(QSize(30,30));
        isBackgroundMusicOn = true;
    }

}

void GameView::on_pushButton_pause_clicked()
{
    if(isGamePaused)    //game is in paused status
    {
        timerForStopWatch->start(100);
        keyEventBlock = false;
        //set game status label
        gameStatusLabelDestructor();

        //set sound properties
        if(isBackgroundMusicOn) //Resume music only when background music is on
        {
            if(timeLeft<=20)
                w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusicHurry,34500);
            else
                w->sound->soundBgMusicPlay(w->sound->gameviewBackgroundMusic,76800);
        }
        ui->pushButton_pause->setIcon(QIcon(":/images/resource/pause.png"));
        ui->pushButton_pause->setIconSize(QSize(30,30));
        isGamePaused = false;
    }
    else    //game is in playing status
    {
        w->sound->soundPlay(w->sound->pauseAlert);
        timerForStopWatch->stop();
        keyEventBlock = true;

        //set game status label
        if(gameStatusLabel == NULL)
            gameStatusLabelCreator();
        gameStatusLabel->setText("Paused\n（￣￢￣）. z Z ");
        gameStatusLabel->setStyleSheet("QLabel{background-color : QColor(0,0,0,60) ; color : yellow}");
        gameAreaScene->addWidget(gameStatusLabel);

        //set sound properties
        w->sound->soundBgMusicPlay_stop();
        ui->pushButton_pause->setIcon(QIcon(":/images/resource/play.png"));
        ui->pushButton_pause->setIconSize(QSize(30,30));
        isGamePaused = true;
    }

}
