#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    this->setWindowTitle("Qt OCR");


    ui->languageSelect->setSortingEnabled(true);
    ui->languageSelect->setSelectionMode(QAbstractItemView::SingleSelection);



    QObject::connect(ui->openFileButton,SIGNAL(clicked()),this,SLOT(chooseFile()));
    QObject::connect(ui->languageSelect,SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));
    QObject::connect(ui->generateOCRButton,SIGNAL(clicked()),this,SLOT(recognize()));
    QObject::connect(ui->exportToFileButton,SIGNAL(clicked()),this,SLOT(saveToFile()));
    QObject::connect(ui->apiKeySubmit, SIGNAL(clicked()), this, SLOT(getApiKey()));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getApiKey() {
    key = ui->apiEdit->text();
}

void MainWindow::chooseFile()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Image Files (*.png *.jpg *.gif);;PDF Files (*.PDF)"));
    if(!fileName.isNull()) {
       originalImage->load(fileName);
    }
    else {
        std::cout<<"failed to open file"<<std::endl;
    }
}


void MainWindow::saveToFile() {
    QString saveAsName = QFileDialog::getSaveFileName(this, tr("Save As"), QDir::homePath(), tr("PlainText file (*.txt)"));
    if(!saveAsName.isNull()) {
        QFile outputFile(saveAsName);
        outputFile.open(QFile::WriteOnly);
        QTextStream out(&outputFile);
        out << text;
        outputFile.close();
    }
    else {
       std::cout << "file not saved" << std::endl;
    }
}


QHttpPart partParameter(QString key, QString value) {
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\""+ key +"\""));
    part.setBody(value.toLatin1());
    return part;
}

void MainWindow::recognize() {
    multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    //set up http part message to send to api that contains image data
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/gif"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"*.gif\""));

    QFile* file = new QFile(fileName);

    //debugging: make sure file was uploaded
    if(!file->open(QIODevice::ReadOnly)) {
        std::cout << "# Could not upload/open file" << std::endl;
    }

    QByteArray fileContent(file->readAll());
    imagePart.setBody(fileContent);

    //append image data, api key, language, and overlay setting to multipart
    multipart->append(imagePart);
    multipart->append(partParameter("language", language));

    //alert user if no language was chosen
    if(language == "") {
        QMessageBox* message = new QMessageBox(this);
        message->setText("Please select a language");
        message->exec();
    }
    if(key == "") {
        QMessageBox* message = new QMessageBox(this);
        message->setText("Please enter an API key");
        message->exec();
    }
    multipart->append(partParameter("apikey",key));

    //OCR Space API url
    QUrl api_url("https://api.ocr.space/parse/image");

    //create network request obj that contains the api url
    QNetworkRequest api_request(api_url);
    manager = new QNetworkAccessManager;
    reply = manager->post(api_request, multipart);

    QObject::connect(reply, SIGNAL(finished()), this, SLOT(networkData()));

    imagePart.setBodyDevice(file);
    file->setParent(multipart);
    networkData();
}


void MainWindow::onItemClicked(QListWidgetItem *item) {
    QString text = QString("%1").arg(item->text());
    language = text.mid(0,3).toLower();
}

void MainWindow::networkData() {
    //test for network error
     QNetworkReply::NetworkError err = reply->error();
     if (err != QNetworkReply::NoError) {
         return;
     }

    //store the network's response in a string
    QString response = (QString)reply->readAll();

    //network reply is stored in JSON format; get only the OCR'd text results
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray jsonArr = jsonObj["ParsedResults"].toArray();
    foreach(const QJsonValue& value, jsonArr) {
        QJsonObject obj = value.toObject();
        text.append(obj["ParsedText"].toString());
    }

    ui->displayText->setText(text);
}


