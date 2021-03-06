#include "trackinfo.h"

// #define debug_info

TrackInfo::TrackInfo(QObject *parent) : QObject(parent)
{
    connect(&dba, SIGNAL(error(QString)), this, SLOT(errorFromDBA(QString)));
}

void TrackInfo::errorFromDBA(QString msg)
{
    qInfo() << "Error from dba: " << msg;
    emit error(msg);
}

void TrackInfo::setAssetNum(QString _assetNum)
{
    qInfo() << "AssetNum: " << _assetNum;
    if(_assetNum.isEmpty() == true)
    {
        qInfo() << "Error: assetNum is empty";
        emit error("Ошибка: код пути пуст.");
        return;
    }

//    if(_assetNum.size() <= 5)
//    {
//        qInfo() << "Error: assetNum <= 5";
//        emit error("Ошибка: некорректный код пути (" + _assetNum + "). Отредактируйте дерево проездов.");
//        return;
//    }
    QMap<QString, QVariant> bindValue;
    bindValue.insert(":ASSET_NUM", _assetNum);



    QSqlQuery query = dba.execQueryFileBind(sqlPath + "/ASSETNUM.sql",
                                            bindValue);

    if(query.size() == 0)
    {
        qInfo() << "Error: assetNum isn't exist";
        emit error("Ошибка: код пути " + _assetNum + " не был найден в базе данных.");
        return;
    }
    query.first();
    setDirInfo(query.value("KOD").toString(), query.value("PUT").toString());

    qInfo() << dirCode << " " << trackNum;
}

void TrackInfo::setDirInfo(QString _dirCode, QString _trackNum)
{
    qInfo() << "DirCode: " << _dirCode << ", trackNum: " << _trackNum;
    dirCode = _dirCode;
    trackNum = _trackNum;

    QMap<QString, QVariant> bindValue;
    bindValue.insert(":DIR_CODE", dirCode);
    QSqlQuery query = dba.execQueryFileBind(sqlPath + "/WAY.sql", bindValue);
    if(query.first() == false)
    {
        qInfo() << "Error while getting dirName";
        qInfo() << query.lastError();
        emit error("Ошибка при выполнении запроса для конвертации кода пути в код направления и номер пути (" +
                   _dirCode + ", " + _trackNum + "\nТекст ошибки: " + query.lastError().text());
    }

    dirName = query.value("NAME").toString();
}

bool TrackInfo::setAndOpenDatabase(QString databaseName, QString _sqlPath)
{
    if(dba.openDatabase(databaseName) == false)
    {
        qInfo() << "Error: database error";
        qInfo() << dba.lastError();
        // emit error("Ошибка при открытии БД: " + dba.lastError());
        return false;
    }

    sqlPath = _sqlPath;
    return true;
}

QVector<TrackItem> TrackInfo::getVec(QString sqlName)
{
    QTime time = QTime::currentTime();
    QVector<TrackItem> result;
    QMap<QString, QVariant> bindValue;
    qInfo() << "Sql path: " << sqlName;
    if(dirCode.isEmpty() && trackNum.isEmpty())
    {
        qInfo() << "dir Code && track Num is empty";
        if(assetNum.isEmpty() == true)
        {
            qInfo() << "Asset num is empty";
            // emit error("Отсутствует код пути и код направления.");
            return result;
        }
        bindValue.insert(":ASSET_NUM", assetNum);
    }
    else
        bindValue.insert(":DIR_CODE", dirCode);
        bindValue.insert(":TRACK_NUM", trackNum);

    QSqlQuery query = dba.execQueryFileBind(sqlPath +
                                            sqlName, bindValue);

    if(query.first() == false)
    {
        qInfo() << sqlName << " - нет данных";
        return QVector<TrackItem>();
    }
    do
    {
        TrackItem item;
        QString objType = query.value("OBJ").toString();
        if(objType == "NKM")
        {
            item.type = TrackItem::KM;
            item.km = query.value("NUMB").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endM = query.value("E_M").toInt();
            item.len = query.value("LEN").toInt();
        }
        else if(objType == "STR")
        {
            item.type = TrackItem::STR;
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.name = query.value("NAME").toString();
            item.numb = query.value("NUMB").toString();
        }
        else if(objType == "STN")
        {
            item.type = TrackItem::STAN;
            item.name = query.value("NAME").toString();
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.oKM = query.value("O_KM").toInt();
            item.oM = query.value("O_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();
        }
        else if(objType == "PCH")
        {
            item.type = TrackItem::PCH;
            item.name = query.value("type").toString() + " - " + query.value("NUMB").toString();
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();
        }
        else if(objType == "MST")
        {
            item.type = TrackItem::MOST;
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();

        }
        else if(objType == "MOV")
        {
            item.type = TrackItem::MOV;
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();

        }
        else if(objType == "OVR")
        {
            item.type = TrackItem::OVR;
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();
        }
        else if(objType == "TNL")
        {
            item.type = TrackItem::TNL;
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();
        }
        else if(objType == "CUR")
        {
            item.type = TrackItem::CUR;
            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.oKM = query.value("O_KM").toInt();
            item.oM = query.value("O_M").toInt();
        }
        else if(objType == "SPD")
        {
            item.type = TrackItem::SPD;

            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();


            QString VVSK = query.value("VVSK").toString();
            QString VSK = query.value("VSK").toString();
            QString VSTR = query.value("VSTR").toString();
            QString VALL = query.value("VALL").toString();
            item.name = (VVSK == "0" ? "" : ("С" + VVSK + "/")) +
                        (VSK == "0" ? "" : ("Л" + VSK)) +
                        (VSTR == "0" ? "" : ("Ст" + VSTR + "/")) +
                        (VALL == "0" ? "" : ("А" + VALL)) +
                        ";" +
                        query.value("VPS").toString() + "/" +
                        query.value("VGR").toString() + "/" +
                        query.value("VPR").toString();

        }
        else if(objType == "SLP")
        {
            item.type = TrackItem::SLEEPER;

            item.name = query.value("NAME").toString();

            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();

        }
        else if(objType == "BND")
        {
            item.type = TrackItem::BONDING;

            item.numb = query.value("NUMB").toString();

            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();
        }
        else if(objType == "RAL")
        {
            item.type = TrackItem::RAIL;

            item.numb = query.value("NUMB").toString();

            item.beginKM = query.value("B_KM").toInt();
            item.beginM = query.value("B_M").toInt();
            item.endKM = query.value("E_KM").toInt();
            item.endM = query.value("E_M").toInt();
        }
        else if(objType == "ISO")
        {
            item.type = TrackItem::ISO;

            item.oKM = query.value("O_KM").toInt();
            item.oM = query.value("O_M").toInt();
        }

        result.push_back(item);
    } while(query.next());


    qInfo() << "Query time: " << time.msecsTo(QTime::currentTime()) << " ms";
    qInfo() << "Query  " << sqlName << " size: " << result.size();

    return result;


}

QMap<TrackItem::TrackItemType, QVector<TrackItem>> TrackInfo::getItemsMap()
{
    QMap<TrackItem::TrackItemType, QVector<TrackItem>> itemsMap;


    QVector<TrackItem> km = getVec("/NKM.sql");
    calculateAbsCoordForKm(km);

    QVector<TrackItem> str = getVec( "/STR.sql");
    calculateAbsCoord(str, km);
    QVector<TrackItem> stan =  getVec( "/STAN.sql");
    calculateAbsCoord(stan, km);
    QVector<TrackItem> pch =  getVec( "/PCH.sql");
    calculateAbsCoord(pch, km);
    QVector<TrackItem> most =  getVec( "/MOST.sql");
    calculateAbsCoord(most, km);
    QVector<TrackItem> mov =  getVec( "/MOV.sql");
    calculateAbsCoord(mov, km);
    QVector<TrackItem> ovr =  getVec( "/OVERPASS.sql");
    calculateAbsCoord(ovr, km);
    QVector<TrackItem> tnl =  getVec( "/TUNNEL.sql");
    calculateAbsCoord(tnl, km);
    QVector<TrackItem> cur =  getVec( "/CUR.sql");
    calculateAbsCoord(cur, km);
    QVector<TrackItem> spd =  getVec( "/SPD.sql");
    calculateAbsCoord(spd, km);
    QVector<TrackItem> sleeper = getVec("/SLEEPER.sql");
    calculateAbsCoord(sleeper, km);
    QVector<TrackItem> bonding = getVec("/BONDING.sql");
    calculateAbsCoord(bonding, km);
    QVector<TrackItem> rail = getVec("/RAIL.sql");
    calculateAbsCoord(rail, km);
    QVector<TrackItem> iso = getVec("/ISO.sql");
    calculateAbsCoord(iso, km);

//    std::sort(str.begin(), str.end());
//    std::sort(cur.begin(), cur.end());

    QTime time = QTime::currentTime();
    itemsMap.insert(TrackItem::KM, km);
    itemsMap.insert(TrackItem::STR, str);
    itemsMap.insert(TrackItem::STAN, stan);
    itemsMap.insert(TrackItem::PCH, pch);
    itemsMap.insert(TrackItem::MOST, most);
    itemsMap.insert(TrackItem::MOV, mov);
    itemsMap.insert(TrackItem::OVR, ovr);
    itemsMap.insert(TrackItem::TNL, tnl);
    itemsMap.insert(TrackItem::CUR, cur);
    itemsMap.insert(TrackItem::SPD, spd);
    itemsMap.insert(TrackItem::SLEEPER, sleeper);
    itemsMap.insert(TrackItem::BONDING, bonding);
    itemsMap.insert(TrackItem::RAIL, rail);
    itemsMap.insert(TrackItem::ISO, iso);
    qInfo() << "Time for insert: " << time.msecsTo(QTime::currentTime());
    return itemsMap;
}

void TrackInfo::calculateAbsCoordForKm(QVector<TrackItem> &km)
{

    QTime time = QTime::currentTime();
    qInfo() << "Calculate Abs Coord For Km";

    int absCoord = -1;

    for(auto it = km.begin(); it != km.end(); ++it)
    {
        it->absBegin = ++absCoord;
        absCoord += it->len;
        it->absEnd = absCoord;
#ifdef debug_info
        qInfo() << "Km: " << it->km << "; Begin: " << it->beginM << "/" << it->absBegin << "; End: " << it->endM << "/" << it->absEnd << "; Len: " << it->len;
    #endif
    }
    qInfo() << "Time km: " << time.msecsTo(QTime::currentTime());
}

void TrackInfo::calculateAbsCoord(QVector<TrackItem> &trackItems, QVector<TrackItem> &km)
{
    if(trackItems.isEmpty())
    {
        qInfo() << "Нет данных для вычисления абсолютной координаты";
        return;
    }
    qInfo() << "Calculate Abs Coord";
    QTime time = QTime::currentTime();
    if(trackItems.first().type == TrackItem::STR)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
#ifdef debug_info
            qInfo() << "Str: Begin: " << it->beginKM << "km " << it->beginM
                     << " Abs Begin: " << it->absBegin << " numb: " << it->numb;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::STAN)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absO = getAbsCoord(km, it->oKM, it->oM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
             qInfo() << "Stan: " << it->name << "; Begin" << it->beginKM << "km " << it->beginM << "m; " << "Os: " << it->oKM << " km " << it->oM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                    << " Abs Begin: " << it->absBegin << "; Abs Os: " << it->absO << "; Abs End: " << it->absEnd;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::PCH)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
             qInfo() << "Pch: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::MOST)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Most: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::MOV)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Mov: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::OVR)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Overpass: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::TNL)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Tunnel: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::CUR)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
#ifdef debug_info
             qInfo() << "Cur: Begin" << it->beginKM << "km " << it->beginM << "m;"
                     << " Abs Begin: " << it->absBegin;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::SPD)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Spd: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd << " spd: " << it->name;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::SLEEPER)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Sleeper: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd << "; name: " << it->name;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::BONDING)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Bonding: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd << "; numb: " << it->numb;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::RAIL)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absBegin = getAbsCoord(km, it->beginKM, it->beginM);
            it->absEnd = getAbsCoord(km, it->endKM, it->endM);
#ifdef debug_info
            qInfo() << "Rail: Begin" << it->beginKM << "km " << it->beginM << "m; End: " << it->endKM << " km " << it->endM << " m; "
                     << " Abs Begin: " << it->absBegin  << "; Abs End: " << it->absEnd << "; numb: " << it->numb;
#endif
        }
    }
    else if(trackItems.first().type == TrackItem::ISO)
    {
        for(auto it = trackItems.begin(); it != trackItems.end(); ++it)
        {
            it->absO = getAbsCoord(km, it->oKM, it->oM);
#ifdef debug_info
            qInfo() << "Iso: o: " << it->oKM << "km " << it->oM << "m;"
                     << " Abs: " << it->absO;
#endif
        }
    }

    qInfo() << "Time: " << time.msecsTo(QTime::currentTime());
}


int TrackInfo::getAbsCoord(QVector<TrackItem> &km, int pathKm, int pathM)
{
    int absCoord = -1;
    for(auto it = km.begin(); it != km.end(); ++it)
    {
        if(pathKm == it->km)
        {
            absCoord = it->absBegin - it->beginM + pathM;
            break;
        }
    }

    return absCoord;
}
