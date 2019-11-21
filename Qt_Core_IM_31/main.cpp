#include <QtCore/QCoreApplication>

#include <qdebug.h>
#include <qfile.h>
#include <qdir.h>
#include <qbuffer.h>
#include <qtextstream.h>

// Our own custom file format, will not work with anthing else;

bool makeFile(QString path) {
	QFile file(path);

	if (file.open(QIODevice::WriteOnly)) {

		QByteArray data;

		for (int i = 0; i < 1000; i++) {
			data.append(QString::number(i) + "\r\n");
		}
		file.write(data);

		file.close();
		return true;
	}

	return false;
}

QByteArray getHeader() {
	QByteArray header;
	header.append("@!~!@");
	return header;
}

bool compressFile(QString originalFile, QString newFile) {
	QFile ofile(originalFile);
	QFile nfile(newFile);
	QByteArray header = getHeader();

	if (!ofile.open(QIODevice::ReadOnly)) return false;
	if (!nfile.open(QIODevice::WriteOnly)) return false;

	int size = 1024;

	while (!ofile.atEnd()) {
		QByteArray buffer = ofile.read(size);
		QByteArray compressed = qCompress(buffer);
		qInfo() << "Header at: " << nfile.pos();
		nfile.write(header);
		qInfo() << "Size: " << nfile.write(compressed);	// unknown size
	}
	ofile.close();
	nfile.close();

	return true;
}

bool decompressFile(QString originalFile, QString newFile) {
	QFile ofile(originalFile);
	QFile nfile(newFile);
	QByteArray header = getHeader();

	if (!ofile.open(QIODevice::ReadOnly)) return false;
	if (!nfile.open(QIODevice::WriteOnly)) return false;

	int size = 1024;

	// Double check that WE Compressed the file
	QByteArray buffer = ofile.peek(size);

	if (!buffer.startsWith(header)) {
		qCritical() << "We did not create this file!";
		ofile.close();
		nfile.close();
		return false;
	}

	// Find the header position
	ofile.seek(header.length());
	qInfo() << "Starting at: " << ofile.pos();

	while (!ofile.atEnd()) {
		buffer = ofile.peek(size);					// buffer�� 1024��ŭ �����͸� �ִ´�.
		qint64 index = buffer.indexOf(header);		// buffer���� header�� ã�� index�� �ѱ��.
		qInfo() << "Header found at: " << index;	
		if (index > -1) {							// �ε����� ã�Ҵٸ�
			// Have the header
			qint64 maxbyte = index;					// �ε��� ��ŭ �ִ´�.
			qInfo() << "Reading: " << maxbyte;
			buffer = ofile.read(maxbyte);			// �ε������� �����͸� �о�´�.
			ofile.read(header.length());			// ofile�� pos�� header��ŭ �ű��.
		}
		else {
			// Don't Have the header
			qInfo() << "Read all";
			buffer = ofile.readAll();
		}

		QByteArray decompressed = qUncompress(buffer);
		qInfo() << "Decompressed: " << nfile.write(decompressed);
		nfile.flush();
	}

	ofile.close();
	nfile.close();

	return true;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QString originalFile = QDir::currentPath() + QDir::separator() + "original.txt";
	QString compresseFile = QDir::currentPath() + QDir::separator() + "compressed.txt";
	QString decompressedFile = QDir::currentPath() + QDir::separator() + "decompressed.txt";

	if (makeFile(originalFile)) {
		qInfo() << "Original created!";
		if (compressFile(originalFile, compresseFile)) {
			qInfo() << "File compressed!";

			if (decompressFile(compresseFile, decompressedFile)) {
				qInfo() << "File decompressed!";
			}
			else {
				qInfo() << "Could not decompressed!";
			}
		}
		else {
			qInfo() << "File not compressed!";
		}
	}

	return a.exec();
}
