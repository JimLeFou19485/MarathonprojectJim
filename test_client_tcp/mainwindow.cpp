#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initialisation de l'interface graphique
    ui->setupUi(this);

    // Instanciation de la socket
    tcpSocket = new QTcpSocket(this);

    // Attachement d'un slot qui sera appelé à chaque fois que des données arrivent (mode asynchrone)
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(gerer_donnees()));

    // Idem pour les erreurs
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(afficher_erreur(QAbstractSocket::SocketError)));
    // Créer un objet QTimer
    pTimer = new QTimer(this);

    // Connecter le signal timeout() du timer à la méthode trame_requete()
    connect(pTimer, SIGNAL(timeout()), this, SLOT(trame_requete()));

   //Création de la carte
    pCarte = new QImage();
    pCarte->load(R"(C:\Users\neonast\OneDrive - OGEC Ensemble Scolaire Niortais\Documents\GitHub\Marathonproject\carte_la_rochelle_plan.png)");



}



MainWindow::~MainWindow()
{
    // Destruction de la socket
    tcpSocket->abort();
    delete tcpSocket;

    // Destruction de l'interface graphique
    delete ui;
}

void MainWindow::on_connexionButton_clicked()
{
    // Récupération des paramètres
    QString adresse_ip = ui->lineEdit_ip->text();
    unsigned short port_tcp = ui->lineEdit_port->text().toInt();

    // Connexion au serveur
    tcpSocket->connectToHost(adresse_ip, port_tcp);


}

void MainWindow::on_deconnexionButton_clicked()
{
    // Déconnexion du serveur
    tcpSocket->close();
    // delete timer
    delete pTimer;
    // suppression carte
    delete pCarte;
}

void MainWindow::on_envoiButton_clicked()
{
    // Lancer le timer avec un intervalle de 1000 ms (1 seconde)
    pTimer->start(1000);
    // Préparation de la requête
    QByteArray requete;
    requete = "Bonjour";

    // Envoi de la requête
    tcpSocket->write(requete);
}

void MainWindow::gerer_donnees()
{
    // Réception des données
    QByteArray reponse = tcpSocket->readAll();
    QString trame = QString(reponse);
    QStringList liste = trame.split(",");
    QString horaire= liste[1];
    // Récupérer l'âge depuis l'interface utilisateur
    int age = ui->Age->text().toInt();
    int frequenceCardiaqueMaximale = 220 - age;
    // Récupérer la chaîne de caractères du nombre de satellites
    QString nombreSatellitesStr = liste[7];
    int nombreSatellites = nombreSatellitesStr.toInt(); // Convertir la chaîne en entier

    // Extraction de l'heure minute et seconde
    int heure = horaire.mid(0,2).toInt();
    int minute = horaire.mid(2,2).toInt();
    int seconde = horaire.mid(4,2).toInt();
    int premier_releve = 28957;
    int timestamp = (heure * 3600) + (minute * 60) + seconde;
    QString frequenceCardiaqueStr = liste[15]; // Récupérer la valeur de fréquence cardiaque
    int frequenceCardiaque = frequenceCardiaqueStr.toInt(); // Convertir la chaîne en entier

    // Conversion du timestamp en heures, minutes et secondes
    int heures = timestamp / 3600;
    int minutes = (timestamp % 3600) / 60;
    int secondes = timestamp % 60;
    QString tempsString = QString("%1:%2:%3").arg(heures, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0')).arg(secondes, 2, 10, QLatin1Char('0'));
    // Extraction de l'altitude depuis la trame NMEA (élément 9)
    QString altitudeStr = liste[9];
    double altitude = altitudeStr.toDouble(); // Convertir en double

    // Extraction de la latitude et de la longitude depuis la trame NMEA
    QString latitudeStr = liste[2];
    QString longitudeStr = liste[4];

    // Conversion de la latitude de DDMM.MMMM à degrés décimaux
    double latitude = latitudeStr.left(2).toDouble() + latitudeStr.mid(2).toDouble() / 60.0;

    // Conversion de la longitude de DDDMM.MMMM à degrés décimaux
    double longitude = longitudeStr.left(3).toDouble() + longitudeStr.mid(3).toDouble() / 60.0;

    // Vérifier si la latitude est au sud (S) et la longitude à l'ouest (W) et ajuster les signes en conséquence
    if (liste[3] == "S")
    {
        latitude = -latitude;
    }
    if (liste[5] == "W")
    {
        longitude = -longitude;
    }

    // Affichage
    ui->labelAltitude->setText(QString::number(altitude, 'f', 2)); // Affiche 2 décimales pour l'altitude
    ui->label_Carte->setPixmap(QPixmap::fromImage(*pCarte));
    ui->labelHeure->setText(tempsString);
    ui->lineEdit_reponse->setText(QString(reponse));
    ui->labelLatitude->setText(latitudeStr);
    ui->labelLongitude->setText(longitudeStr);
    ui->label_FCmax->setText(QString("%1 bpm").arg(frequenceCardiaqueMaximale));
    ui->label_nbrSatellite->setText(QString(" %1").arg(nombreSatellites));
    ui->label_FrequenceCardiaque->setText(QString("Fréquence cardiaque : %1 bpm").arg(frequenceCardiaque));


    // Timer

}
void MainWindow::trame_requete(){
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
}
void MainWindow::afficher_erreur(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Hôte introuvable"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Connexion refusée"));
            break;
        default:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Erreur : %1.")
                                     .arg(tcpSocket->errorString()));
    }
}

