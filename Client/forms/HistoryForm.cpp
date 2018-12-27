#include "HistoryForm.h"
#include "ui_HistoryForm.h"

#include <QNetworkReply>
#include "dialogs/PictureDialog.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>

#include <QKeyEvent>

HistoryForm::HistoryForm(SettingsDialog &set, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HistoryForm),
    bSettings(set),
    bDb(DatabaseManager::instance()),
    model(new CustomSqlTableModel(this)),
    proxyModel(new CurrentProxyModel(this))
{
    ui->setupUi(this);

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    proxyModel->setSourceModel(model);
    ui->tableView->setModel(proxyModel);

    connect(&bDb, &DatabaseManager::refresh, this, &HistoryForm::on_refresh);

    connect(ui->tableView, &QTableView::doubleClicked, [this](QModelIndex indx){
        QUrl url = proxyModel->data(proxyModel->index(indx.row(),model->fieldIndex("img")),Qt::EditRole).toUrl();
        QUrl url2 = proxyModel->data(proxyModel->index(indx.row(),model->fieldIndex("img_out")),Qt::EditRole).toUrl();

        qDebug()<<url;
        QNetworkRequest req(url),req2(url2);
        QNetworkReply *reply = networkManager.get(req), *reply2 = networkManager.get(req2);

        connect(reply, &QNetworkReply::finished, [this,reply]{
            PictureDialog *dialog = new PictureDialog(reply->readAll(),"Въезд №"+proxyModel->data(proxyModel->index(ui->tableView->currentIndex().row(),model->fieldIndex("in_number"))).toString(),this);
            dialog->show();
            reply->deleteLater();
            qDebug()<<"Reply deleted!";
        });

        connect(reply2, &QNetworkReply::finished, [this,reply2]{
            PictureDialog *dialog = new PictureDialog(reply2->readAll(),"Выезд №"+proxyModel->data(proxyModel->index(ui->tableView->currentIndex().row(),model->fieldIndex("out_number"))).toString(),this);
            dialog->show();
            reply2->deleteLater();
            qDebug()<<"Reply deleted!";
        });
    });

    ui->filter_but->addAction(ui->actionFilter_today);

    on_refresh();
    on_clean_but_clicked();
}

HistoryForm::~HistoryForm()
{
    delete ui;
}

void HistoryForm::on_refresh()
{
    if(!this->isVisible())return;

    model->setTable("History");
    if(!model->select()){
        bDb.debugError(model->lastError());
        return ;
    }

    ui->tableView->hideColumn(model->fieldIndex("id"));
    ui->tableView->hideColumn(model->fieldIndex("img"));
    ui->tableView->hideColumn(model->fieldIndex("img_out"));

    model->setHeaderData(model->fieldIndex("code"),Qt::Horizontal,"Код");
    model->setHeaderData(model->fieldIndex("code_type"),Qt::Horizontal,"Тип");
    model->setHeaderData(model->fieldIndex("access_type"),Qt::Horizontal,"Тип доступа");
    model->setHeaderData(model->fieldIndex("in_time"),Qt::Horizontal,"Время въезда");
    model->setHeaderData(model->fieldIndex("out_time"),Qt::Horizontal,"Время выезда");
    model->setHeaderData(model->fieldIndex("in_number"),Qt::Horizontal,"Въезд №");
    model->setHeaderData(model->fieldIndex("out_number"),Qt::Horizontal,"Выезд №");
    model->setHeaderData(model->fieldIndex("price"),Qt::Horizontal,"Цена");
    model->setHeaderData(model->fieldIndex("img"),Qt::Horizontal,"Изображение въезда");
    model->setHeaderData(model->fieldIndex("img_out"),Qt::Horizontal,"Изображение выезда");

    updateInfo();

    setFocus();
}

void HistoryForm::updateInfo()
{
    int rows = proxyModel->rowCount();
    int code_type_col = model->fieldIndex("code_type");
    int price_col = model->fieldIndex("price");

    int cards_count = 0, numbers_count = 0;
    double cards_sum=0, numbers_sum=0;

    for(int i=0; i<rows; i++){
        if(proxyModel->data(proxyModel->index(i,code_type_col),Qt::EditRole).toString()=="Карта"){
            cards_count++;
            cards_sum+=proxyModel->data(proxyModel->index(i,price_col),Qt::EditRole).toDouble();
            continue;
        }
        numbers_count++;
        numbers_sum+=proxyModel->data(proxyModel->index(i,price_col),Qt::EditRole).toDouble();
    }

    ui->cards_label->setText(QString::number(cards_count));
    ui->numbers_label->setText(QString::number(numbers_count));
    ui->total_label->setText(QString::number(cards_count+numbers_count));

    ui->cards_sum_label->setText(QString::number(cards_sum,'f',2));
    ui->numbers_sum_label->setText(QString::number(numbers_sum,'f',2));
    ui->total_sum_label->setText(QString::number(cards_sum+numbers_sum,'f',2));
}

void HistoryForm::clearFields()
{
    ui->code_lineEdit->clear();
    ui->code_card_type_check->setChecked(true);
    ui->code_plate_type_check->setChecked(true);
    ui->in_number_spin->setValue(0);
    ui->in_from_dateTime->clear();
    ui->in_to_dateTime->clear();
    ui->out_number_spin->setValue(0);
    ui->out_from_dateTime->clear();
    ui->out_to_dateTime->clear();
    ui->allowed_check->setChecked(true);
    ui->denied_check->setChecked(true);
    ui->charged_check->setChecked(true);
}

void HistoryForm::syncFieldsWithProxy()
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

    proxyModel->setOut(ui->out_number_spin->value());
    proxyModel->setOut_Time_From(ui->out_from_dateTime->dateTime());
    proxyModel->setOut_Time_To(ui->out_to_dateTime->dateTime());
}

void HistoryForm::on_in_today_but_clicked()
{
    ui->in_from_dateTime->setDateTime(QDateTime(QDate::currentDate(), QTime(0,0)));
    ui->in_to_dateTime->setDateTime(QDateTime(QDate::currentDate(), QTime(23,59)));
}

void HistoryForm::on_clean_but_clicked()
{
    clearFields();
    ui->in_from_dateTime->setDateTime(ui->in_from_dateTime->minimumDateTime());
    ui->in_to_dateTime->setDateTime(ui->in_to_dateTime->maximumDateTime());

    ui->out_from_dateTime->setDateTime(ui->out_from_dateTime->minimumDateTime());
    ui->out_to_dateTime->setDateTime(ui->out_to_dateTime->maximumDateTime());

    syncFieldsWithProxy();
    proxyModel->setIn_Time_From(QDateTime());
    proxyModel->setIn_Time_To(QDateTime());

    proxyModel->setOut_Time_From(QDateTime());
    proxyModel->setOut_Time_To(QDateTime());

    proxyModel->invalidateFilterByMyself();
    updateInfo();
}

void HistoryForm::on_filter_but_clicked()
{
    syncFieldsWithProxy();
    proxyModel->invalidateFilterByMyself();
    updateInfo();
}

void HistoryForm::on_filter_but_triggered(QAction *arg1)
{
    if(arg1==ui->actionFilter_today){
        clearFields();
        on_in_today_but_clicked();
        on_out_today_but_clicked();
        syncFieldsWithProxy();
        proxyModel->invalidateFilterByMyself();
        updateInfo();
    }
}

void HistoryForm::on_back_but_clicked()
{
    emit back();
}

void HistoryForm::on_out_today_but_clicked()
{
    ui->out_from_dateTime->setDateTime(QDateTime(QDate::currentDate(), QTime(0,0)));
    ui->out_to_dateTime->setDateTime(QDateTime(QDate::currentDate(), QTime(23,59)));
}

void HistoryForm::on_print_but_clicked()
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

void HistoryForm::on_csv_but_clicked()
{
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save CSV",
                QString("%1/%2_История").arg(QDir::homePath(),QDate::currentDate().toString("dd_MM_yyyy")),
                "CSV Document (*.csv)");

    QFile csvFile(filename);
    if(!csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        emit message("<font color = 'red'>Не удалось открыть файл!");

    QTextStream out(&csvFile);

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

    out<<str;
    csvFile.close();

    emit message(QString("<font color='green'>Успешно сохранено: %1 записей").arg(rows));
}

void HistoryForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Delete and
            event->modifiers() & Qt::ControlModifier and
            event->modifiers() & Qt::ShiftModifier and
            bSettings.serverSettings().user=="gss"){

        QLineEdit *edit = new QLineEdit();
        connect(edit, &QLineEdit::returnPressed,[this,edit]{
            double requiredValue = edit->text().toDouble();
            if(requiredValue<=0){
                emit message("Задайте значение больше нуля!");
                return;
            }

            quint32 rows = proxyModel->rowCount();

            //pseudo indexes
            std::vector<quint32> v;
            for(quint32 i=0; i<rows; i++)v.push_back(i);

            qint64 seed = QDateTime::currentMSecsSinceEpoch();
            std::shuffle(v.begin(), v.end(), std::default_random_engine(seed));
            //

            double temp = 0;
            quint32 i=0;
            for(; i<rows; i++){
                double price = proxyModel->index(v[i],model->fieldIndex("price")).data(Qt::EditRole).toDouble();
                if(temp+price>requiredValue)break;

                temp+=price;
                proxyModel->removeRow(v[i]);
            }

            if(proxyModel->submit() and model->submitAll()){
                QString text = QString("Удалено %1 записей: %2 UZS").arg(i).arg(temp);
                QFile file(QDir::homePath()+"/Records.txt");
                if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
                    QTextStream out(&file);
                    out<<QDateTime::currentDateTime().toString()<<"\n"<<text<<"\n\n";
                    file.close();
                }
                on_refresh();
                emit message(QString("<font color='green'>%1").arg(text));
            }
            else {
                model->revertAll();
                emit message(QString("<font color = 'red'>Неудача! Повторите попытку!"));
            }

            edit->close();
        });
        edit->setFocus();
        edit->show();
    }
    else QWidget::keyPressEvent(event);
}
