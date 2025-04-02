#include "KaZsDialog.h"
#include "ui_KaZsDialog.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>

namespace ParamControl {

KaZsDialog::KaZsDialog(quint16 currentKaNumber, quint16 currentZsNumber, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::KaZsDialog)
    , m_kaNumber(currentKaNumber)
    , m_zsNumber(currentZsNumber)
{
    ui->setupUi(this);
    
    // Загрузка списков КА и ЗС
    loadLists();
    
    // Установка текущих значений
    int kaIndex = ui->kaComboBox->findText(QString::number(currentKaNumber));
    if (kaIndex >= 0) {
        ui->kaComboBox->setCurrentIndex(kaIndex);
    }
    
    int zsIndex = ui->zsComboBox->findText(QString::number(currentZsNumber));
    if (zsIndex >= 0) {
        ui->zsComboBox->setCurrentIndex(zsIndex);
    }
    
    // Подключение сигналов кнопок
    connect(ui->okButton, &QPushButton::clicked, this, &KaZsDialog::onOkClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &KaZsDialog::onCancelClicked);
    
    // Настройка окна
    setWindowTitle("Выбор КА и ЗС");
    setFixedSize(sizeHint());
}

KaZsDialog::~KaZsDialog()
{
    delete ui;
}

quint16 KaZsDialog::getKaNumber() const
{
    return m_kaNumber;
}

quint16 KaZsDialog::getZsNumber() const
{
    return m_zsNumber;
}

void KaZsDialog::onOkClicked()
{
    if (validateSelections()) {
        // Получаем выбранные значения
        bool kaOk = false, zsOk = false;
        m_kaNumber = ui->kaComboBox->currentText().toUShort(&kaOk);
        m_zsNumber = ui->zsComboBox->currentText().toUShort(&zsOk);
        
        if (kaOk && zsOk) {
            accept();
        } else {
            QMessageBox::warning(this, "Ошибка", "Неверный формат номера КА или ЗС.");
        }
    }
}

void KaZsDialog::onCancelClicked()
{
    reject();
}

void KaZsDialog::loadLists()
{
    // Загрузка списка КА
    QStringList kaList;
    QFile kaFile("./data/KA_list.txt");
    if (kaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&kaFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                kaList << line;
            }
        }
        kaFile.close();
    } else {
        // Если файл не найден, добавляем стандартные значения
        kaList << "101" << "102" << "103";
    }
    
    // Загрузка списка ЗС
    QStringList zsList;
    QFile zsFile("./data/ZS_list.txt");
    if (zsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&zsFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                zsList << line;
            }
        }
        zsFile.close();
    } else {
        // Если файл не найден, добавляем стандартные значения
        zsList << "0" << "111" << "222" << "333";
    }
    
    // Заполнение комбобоксов
    ui->kaComboBox->clear();
    ui->kaComboBox->addItems(kaList);
    
    ui->zsComboBox->clear();
    ui->zsComboBox->addItems(zsList);
}

bool KaZsDialog::validateSelections()
{
    // Проверка наличия выбранных значений
    if (ui->kaComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран номер КА.");
        ui->kaComboBox->setFocus();
        return false;
    }
    
    if (ui->zsComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран номер ЗС.");
        ui->zsComboBox->setFocus();
        return false;
    }
    
    // Проверка формата номера КА
    bool kaOk = false;
    quint16 kaNumber = ui->kaComboBox->currentText().toUShort(&kaOk);
    
    if (!kaOk || kaNumber < 100 || kaNumber > 999) {
        QMessageBox::warning(this, "Ошибка", "Номер КА должен быть трехзначным числом (от 100 до 999).");
        ui->kaComboBox->setFocus();
        return false;
    }
    
    // Проверка формата номера ЗС
    bool zsOk = false;
    quint16 zsNumber = ui->zsComboBox->currentText().toUShort(&zsOk);
    
    if (!zsOk || (zsNumber != 0 && (zsNumber < 100 || zsNumber > 999))) {
        QMessageBox::warning(this, "Ошибка", "Номер ЗС должен быть 0 или трехзначным числом (от 100 до 999).");
        ui->zsComboBox->setFocus();
        return false;
    }
    
    return true;
}

} // namespace ParamControl
