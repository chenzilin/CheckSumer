#include <QtWidgets>
#include <QtConcurrent>
#include "checksumer.h"

QString Checksumer::m_filepath = QString();
QList<Split_st> Checksumer::m_splitlist = QList<Split_st>();
quint8 Checksumer::m_status = Checksumer::CHECKSUMER_IDLE;

static const quint64 EVERY_SPLIT_BYTESIZE = (1024 * 1024 * 30);

#define split_roundup(x,n) ((x+n-1)/n)

Checksumer::Checksumer(QObject *parent) :
    QObject(parent),
    m_elapsedtime()
{
    QObject::connect(this, SIGNAL(OpenFileButtonClicked(QString)), this, SLOT(OpenFileProcesser(QString)), Qt::AutoConnection);
    QObject::connect(this, SIGNAL(ChecksumButtonClicked()), this, SLOT(ChecksumProcesser()), Qt::AutoConnection);
}

void Checksumer::threadStarted()
{
}

void Checksumer::OpenFileProcesser(QString filepath)
{
    QFileInfo fileInfo(filepath);

    m_filepath = QString();

    if(fileInfo.isReadable()) {
        if (fileInfo.size() > 0){
            m_filepath = filepath;
            m_status = CHECKSUMER_FILEOPENED;
            m_splitlist.clear();
        }
        else{
            qDebug("OpenFileProcesser::file size error : %lld", fileInfo.size());
        }
    }
    else{
        qDebug("OpenFileProcesser::File is unreadable!!!");
    }
}

void Checksumer::ChecksumProcesser()
{
    if (false == m_filepath.isEmpty()){
        QFileInfo fileInfo(m_filepath);

        if(fileInfo.isReadable()) {
            if (fileInfo.size() > 0){
            }
            else{
                qWarning("ChecksumProcesser::file size error : %lld", fileInfo.size());
                return;
            }
        }
        else{
            qWarning("ChecksumProcesser::File is unreadable!!!");
            return;
        }

        // Start Checksumming
        qint64 ChecksummingTime = 0;
        //QTime time;
        m_elapsedtime.restart();

        m_status = CHECKSUMER_CHECKSUMMING;

        // Prepare the QList
        m_splitlist.clear();

        qint64 filesize = fileInfo.size();

        int splitcount = split_roundup(filesize, EVERY_SPLIT_BYTESIZE);

        if (1 == splitcount){
            emit Checksumer_RangeChangedSignal(0, 1);

            Split_st tempSplit;
            tempSplit.index = 0;
            tempSplit.length = filesize;
            tempSplit.checksum = 0;
            tempSplit.offset = 0;
            tempSplit.checksumer_ptr = this;

            Split_st result = splitChecksum(tempSplit);

            emit Checksumer_ValueChangedSignal(1);
            m_status = CHECKSUMER_COMPLETE;
            ChecksummingTime = m_elapsedtime.elapsed();
            emit Checksumer_ChecksumResultSignal(result.checksum, ChecksummingTime);
        }
        else{
            Split_st tempSplit;
            tempSplit.index = 0;
            tempSplit.length = 0;
            tempSplit.checksum = 0;
            tempSplit.offset = 0;
            tempSplit.checksumer_ptr = this;

            for (int loop = 0; loop < splitcount - 1; loop++){
                tempSplit.index = loop;
                tempSplit.offset = (qint64)(EVERY_SPLIT_BYTESIZE * loop);
                tempSplit.length = EVERY_SPLIT_BYTESIZE;
                m_splitlist.append(tempSplit);
            }

            /* last split */
            tempSplit.index = splitcount - 1;
            tempSplit.offset = (qint64)(EVERY_SPLIT_BYTESIZE * (splitcount - 1));

            qint64 lastsplitsize = filesize%EVERY_SPLIT_BYTESIZE;
            if(0 == lastsplitsize){
                tempSplit.length = EVERY_SPLIT_BYTESIZE;
            }
            else{
                tempSplit.length = lastsplitsize;
            }
            m_splitlist.append(tempSplit);

            emit Checksumer_RangeChangedSignal(0, (splitcount - 1));

            quint64 final_checksum = QtConcurrent::mappedReduced(m_splitlist, Checksumer::splitChecksum, Checksumer::reduceResult, QtConcurrent::ReduceOptions(QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce));

            m_status = CHECKSUMER_COMPLETE;
            ChecksummingTime = m_elapsedtime.elapsed();
            emit Checksumer_ChecksumResultSignal(final_checksum, ChecksummingTime);
        }

        m_elapsedtime.invalidate();
    }
    else{
        qWarning("ChecksumProcesser::File path is empty!!!");
        return;
    }
}

Split_st Checksumer::splitChecksum(const Split_st &split)
{
    Split_st subchecksum;
    subchecksum = split;
    subchecksum.checksum = 0;

    QFile file(m_filepath);

    if(file.open(QIODevice::ReadOnly)) {
        qint64 offset = split.offset;
        if (file.seek(offset)){
            QByteArray filebuffer = file.read(split.length);

            foreach (const char &byte, filebuffer)
            {
                subchecksum.checksum += (quint8)byte;
            }
        }
        else{
            qWarning("seek failed: %lld", offset);
        }

        file.close();
    }

    return subchecksum;
}

void Checksumer::reduceResult(quint64 &checksum, const Split_st &split)
{
    checksum += split.checksum;
    emit split.checksumer_ptr->Checksumer_ValueChangedSignal(split.index);
}

qint64 Checksumer::getElapsedTime()
{
    if (true == m_elapsedtime.isValid()){
        return m_elapsedtime.elapsed();
    }
    else{
        return -1;
    }
}

