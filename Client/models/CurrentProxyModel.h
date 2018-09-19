#ifndef CURRENTPROXYMODEL_H
#define CURRENTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QDateTime>

class CurrentProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CurrentProxyModel(QObject *par=0);

    void setCode(const QString &c){code=c;}

    void setCode_Plate_Type(const bool &c){code_plate_type=c;}
    void setCode_Card_Type(const bool &c){code_card_type=c;}

    void setAccess_Allowed_Type(const bool &c){access_allowed_type=c;}
    void setAccess_Denied_Type(const bool &c){access_denied_type=c;}
    void setAccess_Charged_Type(const bool &c){access_charged_type=c;}

    void setIn(const quint8 &inGate){in=inGate;}
    void setIn_Time_From(const QDateTime &time){in_time_from=time;}
    void setIn_Time_To(const QDateTime &time){in_time_to=time;}

    //history
    void setOut(const quint8 &outGate){out=outGate;}
    void setOut_Time_From(const QDateTime &time){out_time_from=time;}
    void setOut_Time_To(const QDateTime &time){out_time_to=time;}
    //

    void invalidateFilterByMyself(){this->invalidateFilter();}

private:
    bool dateInRange(const QDateTime &dateTime, bool mode) const;

    QDateTime in_time_from;
    QDateTime in_time_to;
    quint8 in;

    //history
    QDateTime out_time_from;
    QDateTime out_time_to;
    quint8 out;
    //

    QString code;

    bool code_card_type;
    bool code_plate_type;

    bool access_allowed_type;
    bool access_denied_type;
    bool access_charged_type;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // CURRENTPROXYMODEL_H
