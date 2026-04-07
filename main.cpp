#include <QGuiApplication>
#include <QImage>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QDebug>
#include <QDateTime>
#include <QVariantMap>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>

void drawMetadata( QImage& img, const QString& metadata, Qt::AlignmentFlag position = Qt::AlignTop ) {

    const int horizontalPadding { 20 };
    const int margim { 20 };
    const int implicityHeight { 50 };
    const int radius { 5 };

    QPainter p(&img);
    p.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );

    QRect rect;

    switch (position) {
    case Qt::AlignBottom:
        rect = QRect( horizontalPadding, img.height() - ( margim + implicityHeight ), img.width() - ( horizontalPadding * 2 ), implicityHeight);
        break;
    default:
        rect = QRect( horizontalPadding, margim, img.width() - ( horizontalPadding * 2 ), implicityHeight);
        break;
    }

    p.setBrush( QBrush( QColor( 0, 0, 0, 40 ) ) );
    p.setPen( QPen( QColor( 0, 0, 0, 100 ), 2 ) );
    p.drawRoundedRect( rect, radius, radius );

    auto font = qApp->font();

    font.setFamily("Arial");

    // Busca o maior tamanho de font que caiba no retângulo entre 10~32 pointSize
    for (int i = 32; i > 10; --i) {
        font.setPointSize( i );
        const QFontMetrics fm(font);

        const int textWidth = fm.horizontalAdvance( metadata );

        if (rect.width() > textWidth) {
            break;
        }
    }

    p.setFont( font );

    p.setPen( QPen( Qt::black, 1 ) );
    p.setBrush( Qt::NoBrush );

    QTextOption options;
    options.setWrapMode(QTextOption::WordWrap);
    options.setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    p.drawText( rect, metadata, options );
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("MyApp");
    QCoreApplication::setApplicationVersion("1.0");

    // Configuração do Parser de Linha de Comando
    QCommandLineParser parser;
    parser.setApplicationDescription("Adiciona metadados visuais em uma imagem.");
    parser.addHelpOption();
    parser.addVersionOption();

    // Opção de entrada: -i ou --input
    QCommandLineOption inputOption(QStringList() << "i" << "input",
                                   "Caminho da imagem de entrada (Obrigatório).",
                                   "inputFile");
    parser.addOption(inputOption);

    // Opção de saída: -o ou --output
    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                    "Caminho da imagem de saída (Opcional).",
                                    "outputFile");
    parser.addOption(outputOption);

    // Opção de mensagem: -m ou --message
    QCommandLineOption messageOption(QStringList() << "m" << "message",
                                     "Mensagem personalizada a ser exibida.",
                                     "messageText", 
                                     "Texto Padrão");
    parser.addOption(messageOption);

    // Opção de empresa: -c ou --company
    QCommandLineOption companyOption(QStringList() << "c" << "company",
                                     "Nome da empresa a ser exibido.",
                                     "companyName", 
                                     "VMI");
    parser.addOption(companyOption);

    // Opção de timestamp (Flag booleana): -dt ou --datetime
    // Note que não há o 3º argumento ("valueName"), fazendo com que funcione como uma flag true/false
    QCommandLineOption dtOption(QStringList() << "dt" << "datetime",
                                "Inclui o timestamp atual na mensagem.");
    parser.addOption(dtOption);

    // Processa os argumentos passados
    parser.process(app);

    // Validação da entrada
    if (!parser.isSet(inputOption)) {
        qWarning() << "Erro: O parâmetro de entrada (-i) é obrigatório.\n";
        parser.showHelp(1);
    }

    QString inputPath = parser.value(inputOption);
    QString outputPath;

    if (parser.isSet(outputOption)) {
        outputPath = parser.value(outputOption);
    } else {
        QFileInfo fileInfo(inputPath);
        outputPath = fileInfo.absolutePath() + "/" + 
                     fileInfo.baseName() + "_modificado." + 
                     fileInfo.completeSuffix();
    }

    QImage img(inputPath);

    if (img.isNull()) {
        qWarning() << "Erro: Nao foi possível carregar a imagem na rota:" << inputPath;
        return -1;
    }

    // Montagem dinâmica da string de metadados
    QStringList metadataParts;
    metadataParts << parser.value(messageOption);

    if (parser.isSet(dtOption)) {
        metadataParts << QDateTime::currentDateTimeUtc().toString("dd MMMM yyyy - hh:mm:ss ap");
    }

    metadataParts << parser.value(companyOption);

    // Junta as partes com o separador " | "
    QString finalMetadataText = metadataParts.join(" | ");

    drawMetadata(img, finalMetadataText, Qt::AlignTop);

    if (!img.save(outputPath)) {
        qWarning() << "Erro: Nao foi possível salvar a imagem na rota:" << outputPath;
        return -1;
    }

    qDebug() << "Sucesso! Imagem salva em:" << outputPath;
    return 0;
}
