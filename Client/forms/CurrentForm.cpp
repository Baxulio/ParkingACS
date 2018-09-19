#include "CurrentForm.h"
#include "ui_CurrentForm.h"

#include <QNetworkReply>
#include "dialogs/PictureDialog.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextStream>
#include <QFile>

CurrentForm::CurrentForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurrentForm),
    bDb(DatabaseManager::instance()),
    model(new CustomSqlTableModel(this)),
    proxyModel(new CurrentProxyModel(this))
{
    ui->setupUi(this);

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    proxyModel->setSourceModel(model);
    ui->tableView->setModel(proxyModel);

    connect(&bDb, &DatabaseManager::refresh, this, &CurrentForm::on_refresh);

    connect(ui->tableView, &QTableView::doubleClicked, [this](QModelIndex indx){
        QUrl url = model->data(model->index(indx.row(),model->fieldIndex("img")),Qt::EditRole).toUrl();
        qDebug()<<url;
        QNetworkRequest req(url);
        QNetworkReply *reply = networkManager.get(req);

        connect(reply, &QNetworkReply::finished, [this,reply]{
            PictureDialog *dialog = new PictureDialog(reply->readAll(),"Въезд №"+proxyModel->data(proxyModel->index(ui->tableView->currentIndex().row(),model->fieldIndex("in_number"))).toString(),this);
            dialog->show();
            reply->deleteLater();
            qDebug()<<"Reply deleted!";
        });
    });

    connect(ui->tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, [](const QPoint &pos){
        qDebug()<<pos;
        //        ui->tableView->indexAt(QPoint(pos.x(), pos.y()+ui->tableView->horizontalHeader()->height()))
    });

    ui->filter_but->addAction(ui->actionFilter_today);
}

CurrentForm::~CurrentForm()
{
    delete ui;
}

void CurrentForm::on_refresh()
{
    if(!this->isVisible())return;

    model->setTable("Active");
    if(!model->select()){
        bDb.debugError(model->lastError());
        return ;
    }

    ui->tableView->hideColumn(model->fieldIndex("id"));
    ui->tableView->hideColumn(model->fieldIndex("img"));

    model->setHeaderData(model->fieldIndex("code"),Qt::Horizontal,"Код");
    model->setHeaderData(model->fieldIndex("code_type"),Qt::Horizontal,"Тип");
    model->setHeaderData(model->fieldIndex("access_type"),Qt::Horizontal,"Тип доступа");
    model->setHeaderData(model->fieldIndex("in_time"),Qt::Horizontal,"Время въезда");
    model->setHeaderData(model->fieldIndex("in_number"),Qt::Horizontal,"Въезд №");
    model->setHeaderData(model->fieldIndex("img"),Qt::Horizontal,"Ссылка на изображение");

    updateInfo();
}

void CurrentForm::updateInfo()
{
    int rows = proxyModel->rowCount();
    int code_type_col = model->fieldIndex("code_type");

    int cards_count = 0;
    int numbers_count = 0;

    for(int i=0; i<rows; i++){
        proxyModel->data(proxyModel->index(i,code_type_col),Qt::EditRole).toString()=="Карта"?
                    cards_count++:numbers_count++;
    }

    ui->cards_label->setText(QString("<span style=\"font-size:10pt; color:#55aaff;\">%1</span>").arg(cards_count));
    ui->numbers_label->setText(QString("<span style=\"font-size:10pt; color:#55aaff;\">~%1</span>").arg(numbers_count));
    ui->total_label->setText(QString("<span style=\"font-size:14pt; color:#55aaff;\">~%1</span>").arg(cards_count+numbers_count));
}

void CurrentForm::clearFields()
{
    ui->code_lineEdit->clear();
    ui->code_card_type_check->setChecked(true);
    ui->code_plate_type_check->setChecked(true);
    ui->in_number_spin->clear();
    ui->in_from_dateTime->clear();
    ui->in_to_dateTime->clear();
    ui->allowed_check->setChecked(true);
    ui->denied_check->setChecked(true);
    ui->charged_check->setChecked(true);
}

void CurrentForm::syncFieldsWithProxy()
{
    proxyModel->setCode(ui->code_lineEdit->text());
    proxyModel->setCode_Card_Type(ui->code_card_type_check->isChecked());
    proxyModel->setCode_Plate_Type(ui->code_plate_type_check->isChecked());

    proxyModel->setAccess_Allowed_Type(ui->allowed_check->isChecked());
    proxyModel->setAccess_Denied_Type(ui->denied_check->isChecked());
    proxyModel->setAccess_Charged_Type(ui->charged_check->isChecked());

    proxyModel->setIn(ui->in_number_spin->value());
    proxyModel->setIn_Time_From(ui->in_from_dateTime->dateTime());
    proxyModel->setIn_Time_To(ui->in_to_dateTime->dateTime());
}

void CurrentForm::on_in_today_but_clicked()
{
    ui->in_from_dateTime->setDateTime(QDateTime(QDate::currentDate(), QTime(0,0)));
    ui->in_to_dateTime->setDateTime(QDateTime(QDate::currentDate(), QTime(23,59)));
}

void CurrentForm::on_clean_but_clicked()
{
    clearFields();
    syncFieldsWithProxy();

    //    ui->in_from_dateTime->setDateTime(ui->in_from_dateTime->minimumDateTime());
    //    proxyModel->setIn_Time_From(QDateTime());
    //    ui->in_to_dateTime->setDateTime(ui->in_to_dateTime->maximumDateTime());
    //    proxyModel->setIn_Time_To(QDateTime());

    proxyModel->invalidateFilterByMyself();
    updateInfo();
}

void CurrentForm::on_filter_but_clicked()
{
    syncFieldsWithProxy();
    proxyModel->invalidateFilterByMyself();
    updateInfo();
}

void CurrentForm::on_filter_but_triggered(QAction *arg1)
{
    if(arg1==ui->actionFilter_today){
        clearFields();
        on_in_today_but_clicked();
        syncFieldsWithProxy();
        proxyModel->invalidateFilterByMyself();
        updateInfo();
    }
}

void CurrentForm::on_back_but_clicked()
{
    emit back();
}

void CurrentForm::on_print_but_clicked()
{
    QString strStream;
    QTextStream out(&strStream);

    const int rowCount = ui->tableView->model()->rowCount();
    const int columnCount = ui->tableView->model()->columnCount();

    out <<  "<html>\n"
            "<head>\n"
            "<meta Content=\"Text/html; charset=Windows-1251\">\n"
         <<  QString("<title>%1 : %2</title>\n").arg("Parking Automated System").arg(QDateTime::currentDateTime().toString())
          <<  "</head>\n"
              "<body bgcolor=#ffffff link=#5000A0>\n"
              "<table border=1 cellspacing=0 cellpadding=2>\n";

    // headers
    out << "<thead><tr bgcolor=#f0f0f0>";
    for (int column = 0; column < columnCount; column++)
        if (!ui->tableView->isColumnHidden(column))
            out << QString("<th>%1</th>").arg(ui->tableView->model()->headerData(column, Qt::Horizontal).toString());
    out << "</tr></thead>\n";

    // data table
    for (int row = 0; row < rowCount; row++) {
        out << "<tr>";
        for (int column = 0; column < columnCount; column++) {
            if (!ui->tableView->isColumnHidden(column)) {
                QString data = ui->tableView->model()->data(ui->tableView->model()->index(row, column)).toString().simplified();
                out << QString("<td bkcolor=0>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }
    out <<  "</table>\n"
            "</body>\n"
            "</html>\n";

    QTextDocument document;
    document.setHtml(strStream);
    QPrinter bPrinter;
    QPrintDialog dialog(&bPrinter, NULL);
    if (dialog.exec() == QDialog::Accepted) {
        document.print(&bPrinter);
    }
}

void CurrentForm::on_csv_but_clicked()
{
    QString str;

    int rows = proxyModel->rowCount();
    int cols = proxyModel->columnCount();

    for(int j=1; j<cols; j++){
        str += proxyModel->headerData(j,Qt::Horizontal).toString();
        str +=";";
    }
    str+="\n";

    for (int i=0; i<rows; i++){
        for(int j=1; j<cols; j++){
            str += proxyModel->data(proxyModel->index(i,j)).toString();
            str +=";";
        }
        str+="\n";
    }
    QFile csvFile(QString("%1.csv").arg(QDate::currentDate().toString("dd_MM_yyyy")));
    if(csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QTextStream out(&csvFile);
        out<<str;
        csvFile.close();
    }
}
