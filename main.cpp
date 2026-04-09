#include <QApplication>
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QHeaderView>
#include <QDir>
#include <QRandomGenerator>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <vector>
#include <memory>

// ==================== Структура даты рождения ====================
struct BirthDate {
    int day;
    int month;
    int year;

    BirthDate(int d = 1, int m = 1, int y = 2000) : day(d), month(m), year(y) {}

    QString toString() const {
        return QString("%1.%2.%3")
        .arg(day, 2, 10, QChar('0'))
            .arg(month, 2, 10, QChar('0'))
            .arg(year);
    }
};

// ==================== Родительский класс ====================
class Person {
protected:
    QString lastName;
    QString firstName;
    BirthDate birthDate;

public:
    Person() : lastName(""), firstName(""), birthDate(1, 1, 2000) {}
    Person(const QString& last, const QString& first, const BirthDate& date)
        : lastName(last), firstName(first), birthDate(date) {}
    virtual ~Person() {}

    virtual QString getType() const { return "Person"; }
    virtual QString getField1() const { return lastName; }
    virtual QString getField2() const { return firstName; }
    virtual QString getField3() const { return ""; }
    BirthDate getBirthDate() const { return birthDate; }

    virtual void craft() = 0;

    static Person* createFromLine(const QString& line);
};

// ==================== Наследник 1 ====================
class RuPerson : public Person {
private:
    QString patronymic;

public:
    RuPerson() : Person(), patronymic("") {}
    RuPerson(const QString& last, const QString& first, const QString& patr, const BirthDate& date)
        : Person(last, first, date), patronymic(patr) {}

    QString getType() const override { return "RuPerson"; }
    QString getField1() const override { return lastName; }
    QString getField2() const override { return firstName; }
    QString getField3() const override { return patronymic; }

    void craft() override {
        qDebug() << "Craft called for RuPerson:" << lastName << firstName;
    }
};

// ==================== Наследник 2 ====================
class AmPerson : public Person {
private:
    QString middleName;

public:
    AmPerson() : Person(), middleName("") {}
    AmPerson(const QString& first, const QString& middle, const QString& last, const BirthDate& date)
        : Person(last, first, date), middleName(middle) {}

    QString getType() const override { return "AmPerson"; }
    QString getField1() const override { return firstName; }
    QString getField2() const override { return middleName; }
    QString getField3() const override { return lastName; }

    void craft() override {
        qDebug() << "Craft called for AmPerson:" << firstName << middleName << lastName;
    }
};

// ==================== Фабричный метод ====================
Person* Person::createFromLine(const QString& line) {
    QStringList parts = line.trimmed().split(',');
    if (parts.size() < 7) return nullptr;

    int code = parts[0].toInt();

    if (code == 1) {
        QString last = parts[1].trimmed();
        QString first = parts[2].trimmed();
        QString patronymic = parts[3].trimmed();
        int day = parts[4].toInt();
        int month = parts[5].toInt();
        int year = parts[6].toInt();

        return new RuPerson(last, first, patronymic, BirthDate(day, month, year));
    }
    else if (code == 2) {
        QString first = parts[1].trimmed();
        QString middle = parts[2].trimmed();
        QString last = parts[3].trimmed();
        int month = parts[4].toInt();
        int day = parts[5].toInt();
        int year = parts[6].toInt();

        return new AmPerson(first, middle, last, BirthDate(day, month, year));
    }

    return nullptr;
}

// ==================== Форма пропуска (без Q_OBJECT) ====================
class PassForm : public QDialog {
private:
    Person* person;
    QLabel* imageLabel;
    int imageMode;

public:
    PassForm(Person* p, int mode, QWidget* parent = nullptr)
        : QDialog(parent), person(p), imageMode(mode) {
        setWindowTitle("Пропуск");
        setMinimumSize(500, 300);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* topLayout = new QHBoxLayout();

        imageLabel = new QLabel();
        imageLabel->setFixedSize(120, 120);
        imageLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
        imageLabel->setScaledContents(true);
        loadImage();
        topLayout->addWidget(imageLabel);

        QWidget* infoWidget = new QWidget();
        QFormLayout* infoLayout = new QFormLayout(infoWidget);

        if (person->getType() == "RuPerson") {
            RuPerson* ru = dynamic_cast<RuPerson*>(person);
            infoLayout->addRow("Фамилия:", new QLabel(ru->getField1()));
            infoLayout->addRow("Имя:", new QLabel(ru->getField2()));
            infoLayout->addRow("Отчество:", new QLabel(ru->getField3()));
        } else {
            AmPerson* am = dynamic_cast<AmPerson*>(person);
            infoLayout->addRow("Имя:", new QLabel(am->getField1()));
            infoLayout->addRow("Второе имя:", new QLabel(am->getField2()));
            infoLayout->addRow("Фамилия:", new QLabel(am->getField3()));
        }

        infoLayout->addRow("Дата рождения:", new QLabel(person->getBirthDate().toString()));

        topLayout->addWidget(infoWidget);
        topLayout->addStretch();
        mainLayout->addLayout(topLayout);

        mainLayout->addStretch();

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* cancelBtn = new QPushButton("Отмена");
        QPushButton* printBtn = new QPushButton("Печать");

        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        connect(printBtn, &QPushButton::clicked, this, &QDialog::accept);

        buttonLayout->addStretch();
        buttonLayout->addWidget(cancelBtn);
        buttonLayout->addWidget(printBtn);
        buttonLayout->addStretch();

        mainLayout->addLayout(buttonLayout);

        // Вызываем craft при создании формы
        person->craft();
    }

    void loadImage() {
        QPixmap pixmap(120, 120);
        pixmap.fill(Qt::lightGray);

        switch(imageMode) {
        case 1: {
            QPainter painter(&pixmap);
            painter.setPen(Qt::black);
            painter.drawText(pixmap.rect(), Qt::AlignCenter, "ФОТО");
            break;
        }
        case 2: {
            QDir dir("img_res");
            QStringList filters;
            filters << "*.jpg" << "*.png" << "*.bmp";
            QStringList files = dir.entryList(filters);

            if (!files.isEmpty()) {
                int randomIndex = QRandomGenerator::global()->bounded(files.size());
                pixmap.load("img_res/" + files[randomIndex]);
                if (pixmap.isNull()) {
                    pixmap = QPixmap(120, 120);
                    pixmap.fill(Qt::lightGray);
                    QPainter painter(&pixmap);
                    painter.setPen(Qt::black);
                    painter.drawText(pixmap.rect(), Qt::AlignCenter, "ФОТО");
                }
            }
            break;
        }
        case 3: {
            QString filename = QString("%1_%2.jpg")
            .arg(person->getField1())
                .arg(person->getField2());
            filename = filename.toLower().replace(" ", "_");

            if (QFile::exists("img_res/" + filename)) {
                pixmap.load("img_res/" + filename);
            } else {
                pixmap.fill(Qt::lightGray);
                QPainter painter(&pixmap);
                painter.setPen(Qt::black);
                painter.drawText(pixmap.rect(), Qt::AlignCenter,
                                 person->getField1().left(1) + person->getField2().left(1));
            }
            break;
        }
        }

        imageLabel->setPixmap(pixmap);
    }

    bool shouldRemove() const { return result() == QDialog::Accepted; }
};

// ==================== Главное окно (без Q_OBJECT) ====================
class MainWindow : public QMainWindow {
private:
    QTableWidget* table;
    QPushButton* loadButton;
    QPushButton* loadCustomButton;
    std::vector<std::unique_ptr<Person>> persons;
    int imageMode;

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent), imageMode(2) {
        setWindowTitle("Лабораторная работа 5 - Наследование и полиморфизм");
        setMinimumSize(800, 500);

        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        table = new QTableWidget(this);
        table->setColumnCount(4);
        QStringList headers;
        headers << "Фамилия" << "Имя" << "Отчество/Второе имя" << "Дата рождения";
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setStretchLastSection(true);
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableDoubleClick);

        mainLayout->addWidget(table);

        QHBoxLayout* buttonLayout = new QHBoxLayout();

        loadButton = new QPushButton("Загрузить (фиксированный файл)", this);
        loadCustomButton = new QPushButton("Загрузить (выбрать файл)", this);

        QPushButton* modeButton = new QPushButton("Режим изображений", this);
        QMenu* modeMenu = new QMenu(this);

        QAction* action1 = modeMenu->addAction("Статичная");
        QAction* action2 = modeMenu->addAction("Рандомная из папки");
        QAction* action3 = modeMenu->addAction("Уникальная для каждого");

        connect(action1, &QAction::triggered, this, [this]() {
            imageMode = 1;
            QMessageBox::information(this, "Режим", "Выбран режим: Статичная картинка");
        });
        connect(action2, &QAction::triggered, this, [this]() {
            imageMode = 2;
            QMessageBox::information(this, "Режим", "Выбран режим: Рандомная картинка");
        });
        connect(action3, &QAction::triggered, this, [this]() {
            imageMode = 3;
            QMessageBox::information(this, "Режим", "Выбран режим: Уникальная картинка");
        });

        modeButton->setMenu(modeMenu);

        connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadFixedFile);
        connect(loadCustomButton, &QPushButton::clicked, this, &MainWindow::loadCustomFile);

        buttonLayout->addWidget(loadButton);
        buttonLayout->addWidget(loadCustomButton);
        buttonLayout->addWidget(modeButton);
        buttonLayout->addStretch();

        mainLayout->addLayout(buttonLayout);

        loadTestData();
    }

    void loadTestData() {
        persons.clear();

        persons.push_back(std::unique_ptr<Person>(new RuPerson("Петров", "Иван", "", BirthDate(22, 1, 2001))));
        persons.push_back(std::unique_ptr<Person>(new RuPerson("Петрова", "Елена", "Ивановна", BirthDate(12, 5, 2000))));
        persons.push_back(std::unique_ptr<Person>(new AmPerson("Джеймс", "Джозеф", "Парсонс", BirthDate(24, 3, 1973))));
        persons.push_back(std::unique_ptr<Person>(new RuPerson("Лазарев", "Лев", "", BirthDate(1, 9, 1997))));

        updateTable();
    }

    void loadFixedFile() {
        QFile file("data.txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл data.txt\nСоздан файл с примером данных.");
            createExampleFile();
            file.open(QIODevice::ReadOnly | QIODevice::Text);
        }

        loadFromFile(file);
    }

    void loadCustomFile() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл с данными", "", "Текстовые файлы (*.txt);;Все файлы (*)");

        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл!");
            return;
        }

        loadFromFile(file);
    }

    void loadFromFile(QFile& file) {
        persons.clear();

        QTextStream stream(&file);
        int lineNum = 0;

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            lineNum++;

            if (line.trimmed().isEmpty()) continue;

            Person* person = Person::createFromLine(line);
            if (person) {
                persons.push_back(std::unique_ptr<Person>(person));
            } else {
                qDebug() << "Ошибка в строке" << lineNum << ":" << line;
            }
        }

        file.close();

        if (persons.empty()) {
            QMessageBox::warning(this, "Предупреждение", "Не удалось загрузить данные из файла!\nЗагружены тестовые данные.");
            loadTestData();
        } else {
            updateTable();
            QMessageBox::information(this, "Успех", QString("Загружено %1 записей").arg(persons.size()));
        }
    }

    void createExampleFile() {
        QFile file("data.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "1, Петров, Иван, , 22, 1, 2001\n";
            out << "1, Петрова, Елена, Ивановна, 12, 5, 2000\n";
            out << "2, Джеймс, Джозеф, Парсонс, 3, 24, 1973\n";
            out << "1, Лазарев, Лев, , 1, 9, 1997\n";
            file.close();
        }
    }

    void updateTable() {
        table->setRowCount(persons.size());

        for (size_t i = 0; i < persons.size(); i++) {
            Person* p = persons[i].get();

            QTableWidgetItem* item1 = new QTableWidgetItem(p->getField1());
            QTableWidgetItem* item2 = new QTableWidgetItem(p->getField2());
            QTableWidgetItem* item3 = new QTableWidgetItem(p->getField3());
            QTableWidgetItem* item4 = new QTableWidgetItem(p->getBirthDate().toString());

            item1->setTextAlignment(Qt::AlignCenter);
            item2->setTextAlignment(Qt::AlignCenter);
            item3->setTextAlignment(Qt::AlignCenter);
            item4->setTextAlignment(Qt::AlignCenter);

            table->setItem(i, 0, item1);
            table->setItem(i, 1, item2);
            table->setItem(i, 2, item3);
            table->setItem(i, 3, item4);
        }

        table->resizeColumnsToContents();
    }

    void onTableDoubleClick(int row, int column) {
        Q_UNUSED(column);

        if (row >= 0 && row < static_cast<int>(persons.size())) {
            PassForm form(persons[row].get(), imageMode, this);

            if (form.exec() == QDialog::Accepted && form.shouldRemove()) {
                persons.erase(persons.begin() + row);
                updateTable();
                QMessageBox::information(this, "Печать", "Пропуск напечатан, человек удален из списка.");
            }
        }
    }
};

// ==================== main ====================
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QDir().mkdir("img_res");

    MainWindow window;
    window.show();

    return app.exec();
}

...............
#include <QApplication>
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QHeaderView>
#include <QDir>
#include <QRandomGenerator>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <vector>
#include <memory>

// ==================== Структура даты рождения ====================
struct BirthDate {
    int day;
    int month;
    int year;

    BirthDate(int d = 1, int m = 1, int y = 2000) : day(d), month(m), year(y) {}

    QString toString() const {
        return QString("%1.%2.%3")
            .arg(day, 2, 10, QChar('0'))
            .arg(month, 2, 10, QChar('0'))
            .arg(year);
    }
};

// ==================== Родительский класс ====================
class Person {
protected:
    QString lastName;
    QString firstName;
    BirthDate birthDate;

public:
    Person() : lastName(""), firstName(""), birthDate(1, 1, 2000) {}
    Person(const QString& last, const QString& first, const BirthDate& date)
        : lastName(last), firstName(first), birthDate(date) {}
    virtual ~Person() {}

    virtual QString getType() const { return "Person"; }
    virtual QString getField1() const { return lastName; }
    virtual QString getField2() const { return firstName; }
    virtual QString getField3() const { return ""; }
    BirthDate getBirthDate() const { return birthDate; }

    virtual void craft() = 0;

    static Person* createFromLine(const QString& line);
};

// ==================== Наследник 1 ====================
class RuPerson : public Person {
private:
    QString patronymic;

public:
    RuPerson() : Person(), patronymic("") {}
    RuPerson(const QString& last, const QString& first, const QString& patr, const BirthDate& date)
        : Person(last, first, date), patronymic(patr) {}

    QString getType() const override { return "RuPerson"; }
    QString getField1() const override { return lastName; }
    QString getField2() const override { return firstName; }
    QString getField3() const override { return patronymic; }

    void craft() override {
        qDebug() << "Craft called for RuPerson:" << lastName << firstName;
    }
};

// ==================== Наследник 2 ====================
class AmPerson : public Person {
private:
    QString middleName;

public:
    AmPerson() : Person(), middleName("") {}
    AmPerson(const QString& first, const QString& middle, const QString& last, const BirthDate& date)
        : Person(last, first, date), middleName(middle) {}

    QString getType() const override { return "AmPerson"; }
    QString getField1() const override { return firstName; }
    QString getField2() const override { return middleName; }
    QString getField3() const override { return lastName; }

    void craft() override {
        qDebug() << "Craft called for AmPerson:" << firstName << middleName << lastName;
    }
};

// ==================== Фабричный метод ====================
Person* Person::createFromLine(const QString& line) {
    QStringList parts = line.trimmed().split(',');
    if (parts.size() < 7) return nullptr;

    int code = parts[0].toInt();

    if (code == 1) {
        QString last = parts[1].trimmed();
        QString first = parts[2].trimmed();
        QString patronymic = parts[3].trimmed();
        int day = parts[4].toInt();
        int month = parts[5].toInt();
        int year = parts[6].toInt();

        return new RuPerson(last, first, patronymic, BirthDate(day, month, year));
    } else if (code == 2) {
        QString first = parts[1].trimmed();
        QString middle = parts[2].trimmed();
        QString last = parts[3].trimmed();
        int month = parts[4].toInt();
        int day = parts[5].toInt();
        int year = parts[6].toInt();

        return new AmPerson(first, middle, last, BirthDate(day, month, year));
    }

    return nullptr;
}

// ==================== Форма пропуска (без Q_OBJECT) ====================
class PassForm : public QDialog {
private:
    Person* person;
    QLabel* imageLabel;
    int imageMode;

public:
    PassForm(Person* p, int mode, QWidget* parent = nullptr)
        : QDialog(parent), person(p), imageMode(mode) {
        setWindowTitle("Пропуск");
        setMinimumSize(500, 300);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* topLayout = new QHBoxLayout();

        imageLabel = new QLabel();
        imageLabel->setFixedSize(120, 120);
        imageLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
        imageLabel->setScaledContents(true);
        loadImage();
        topLayout->addWidget(imageLabel);

        QWidget* infoWidget = new QWidget();
        QFormLayout* infoLayout = new QFormLayout(infoWidget);

        if (person->getType() == "RuPerson") {
            RuPerson* ru = dynamic_cast<RuPerson*>(person);
            infoLayout->addRow("Фамилия:", new QLabel(ru->getField1()));
            infoLayout->addRow("Имя:", new QLabel(ru->getField2()));
            infoLayout->addRow("Отчество:", new QLabel(ru->getField3()));
        } else {
            AmPerson* am = dynamic_cast<AmPerson*>(person);
            infoLayout->addRow("Имя:", new QLabel(am->getField1()));
            infoLayout->addRow("Второе имя:", new QLabel(am->getField2()));
            infoLayout->addRow("Фамилия:", new QLabel(am->getField3()));
        }

        infoLayout->addRow("Дата рождения:", new QLabel(person->getBirthDate().toString()));

        topLayout->addWidget(infoWidget);
        topLayout->addStretch();
        mainLayout->addLayout(topLayout);

        mainLayout->addStretch();

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* cancelBtn = new QPushButton("Отмена");
        QPushButton* printBtn = new QPushButton("Печать");

        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        connect(printBtn, &QPushButton::clicked, this, &QDialog::accept);

        buttonLayout->addStretch();
        buttonLayout->addWidget(cancelBtn);
        buttonLayout->addWidget(printBtn);
        buttonLayout->addStretch();

        mainLayout->addLayout(buttonLayout);

        // Вызываем craft при создании формы
        person->craft();
    }

    void loadImage() {
        QPixmap pixmap(120, 120);
        pixmap.fill(Qt::lightGray);

        switch (imageMode) {
        case 1: {
            QPainter painter(&pixmap);
            painter.setPen(Qt::black);
            painter.drawText(pixmap.rect(), Qt::AlignCenter, "ФОТО");
            break;
        }
        case 2: {
            QDir dir("img_res");
            QStringList filters;
            filters << "*.jpg" << "*.png" << "*.bmp";
    QStringList files = dir.entryList(filters);


            if (!files.isEmpty()) {
                int randomIndex = QRandomGenerator::global()->bounded(files.size());
                pixmap.load("img_res/" + files[randomIndex]);
                if (pixmap.isNull()) {
                    pixmap = QPixmap(120, 120);
            pixmap.fill(Qt::lightGray);
            QPainter painter(&pixmap);
            painter.setPen(Qt::black);
            painter.drawText(pixmap.rect(), Qt::AlignCenter, "ФОТО");
                }
            }
            break;
        }
        case 3: {
            QString filename = QString("%1_%2.jpg")
                .arg(person->getField1())
                .arg(person->getField2());
            filename = filename.toLower().replace(" ", "_");

            if (QFile::exists("img_res/" + filename)) {
                pixmap.load("img_res/" + filename);
            } else {
                pixmap.fill(Qt::lightGray);
                QPainter painter(&pixmap);
                painter.setPen(Qt::black);
                painter.drawText(pixmap.rect(), Qt::AlignCenter,
                    person->getField1().left(1) + person->getField2().left(1));
            }
            break;
        }
        }

        imageLabel->setPixmap(pixmap);
    }

    bool shouldRemove() const { return result() == QDialog::Accepted; }
};

// ==================== Главное окно (без Q_OBJECT) ====================
class MainWindow : public QMainWindow {
private:
    QTableWidget* table;
    QPushButton* loadButton;
    QPushButton* loadCustomButton;
    std::vector<std::unique_ptr<Person>> persons;
    int imageMode;

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent), imageMode(2) {
        setWindowTitle("Лабораторная работа 5 - Наследование и полиморфизм");
        setMinimumSize(800, 500);

        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        table = new QTableWidget(this);
        table->setColumnCount(4);
        QStringList headers;
        headers << "Фамилия" << "Имя" << "Отчество/Второе имя" << "Дата рождения";
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setStretchLastSection(true);
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableDoubleClick);

        mainLayout->addWidget(table);

        QHBoxLayout* buttonLayout = new QHBoxLayout();

        loadButton = new QPushButton("Загрузить (фиксированный файл)", this);
        loadCustomButton = new QPushButton("Загрузить (выбрать файл)", this);

        QPushButton* modeButton = new QPushButton("Режим изображений", this);
        QMenu* modeMenu = new QMenu(this);

        QAction* action1 = modeMenu->addAction("Статичная");
        QAction* action2 = modeMenu->addAction("Рандомная из папки");
        QAction* action3 = modeMenu->addAction("Уникальная для каждого");

        connect(action1, &QAction::triggered, this, [this]() {
            imageMode = 1;
            QMessageBox::information(this, "Режим", "Выбран режим: Статичная картинка");
        });
        connect(action2, &QAction::triggered, this, [this]() {
            imageMode = 2;
            QMessageBox::information(this, "Режим", "Выбран режим: Рандомная картинка");
        });
        connect(action3, &QAction::triggered, this, [this]() {
            imageMode = 3;
            QMessageBox::information(this, "Режим", "Выбран режим: Уникальная картинка");
        });

        modeButton->setMenu(modeMenu);

        connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadFixedFile);
        connect(loadCustomButton, &QPushButton::clicked, this, &MainWindow::loadCustomFile);

        buttonLayout->addWidget(loadButton);
        buttonLayout->addWidget(loadCustomButton);
        buttonLayout->addWidget(modeButton);
        buttonLayout->addStretch();

        mainLayout->addLayout(buttonLayout);

        loadTestData();
    }

private slots:
    void onTableDoubleClick(int row, int column) {
        if (row < 0 || row >= static_cast<int>(persons.size())) return;

        Person* selectedPerson = persons[static_cast<size_t>(row)].get();
        PassForm form(selectedPerson, imageMode, this);
        form.exec();
    }

public:
    void loadTestData() {
        persons.clear();

        persons.push_back(std::unique_ptr<Person>(new RuPerson("Петров", "Иван", "", BirthDate(22, 1, 2001)));
        persons.push_back(std::unique_ptr<Person>(new RuPerson("Петрова", "Елена", "Ивановна", BirthDate(12, 5, 2000)));
        persons.push_back(std::unique_ptr<Person>(new AmPerson("Джеймс", "Джозеф", "Парсонс", BirthDate(24, 3, 1973)));
        persons.push_back(std::unique_ptr<Person>(new RuPerson("Лазарев", "Лев", "", BirthDate(1, 9, 1997)));


        updateTable();
    }

    void loadFixedFile() {
        QFile file("data.txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл data.txt\nСоздан файл с примером данных.");
            createExampleFile();
            file.open(QIODevice::ReadOnly | QIODevice::Text);
        }

        loadFromFile(file);
    }

    void loadCustomFile() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл с данными", "", "Текстовые файлы (*.txt);;Все файлы (*)");

        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл!");
            return;
        }

        loadFromFile(file);
    }

    void loadFromFile(QFile& file) {
        persons.clear();

        QTextStream stream(&file);
        int lineNum = 0;

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            lineNum++;

            if (line.trimmed().isEmpty()) continue;

            Person* person = Person::createFromLine(line);
            if (person) {
                persons.push_back(std::unique_ptr<Person>(person));
            } else {
                qDebug() << "Ошибка в строке" << lineNum << ":" << line;
            }
        }

        file.close();

        if (persons.empty()) {
            QMessageBox::warning(this, "Предупреждение", "Не удалось загрузить данные из файла!\nЗагружены тестовые данные.");
            loadTestData();
        } else {
            updateTable();
            QMessageBox::information(this, "Успех", QString("Загружено %1 записей").arg(persons.size()));
        }
    }

    void createExampleFile() {
        QFile file("data.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "1, Петров, Иван, , 22, 1, 2001\n";
            out << "1, Петрова, Елена, Ивановна, 12, 5, 2000\n";
            out << "2, Джеймс, Джозеф, Парсонс, 3, 24, 1973\n";
            out << "1, Лазарев, Лев, , 1, 9,1997\n";
            out << "1, Сидоров, Алексей, Петрович, 3, 12, 1995\n";
            out << "2, Emily, Rose, Johnson, 20, 7, 1992\n"; // Пятая запись
            out << "1, Козлова, Мария, Сергеевна, 15, 4, 1988\n"; // Шестая запись
            file.close();
        }
    }

    void updateTable() {
        table->setRowCount(static_cast<int>(persons.size()));

        for (size_t i = 0; i < persons.size(); ++i) {
            Person* p = persons[i].get();

            QTableWidgetItem* item1 = new QTableWidgetItem(p->getField1());
            QTableWidgetItem* item2 = new QTableWidgetItem(p->getField2());
            QTableWidgetItem* item3 = new QTableWidgetItem(p->getField3());
            QTableWidgetItem* item4 = new QTableWidgetItem(p->getBirthDate().toString());

            item1->setTextAlignment(Qt::AlignCenter);
            item2->setTextAlignment(Qt::AlignCenter);
            item3->setTextAlignment(Qt::AlignCenter);
            item4->setTextAlignment(Qt::AlignCenter);

            table->setItem(static_cast<int>(i), 0, item1);
            table->setItem(static_cast<int>(i), 1, item2);
            table->setItem(static_cast<int>(i), 2, item3);
            table->setItem(static_cast<int>(i), 3, item4);
        }

        table->resizeColumnsToContents();
    }
};

// ==================== Главная функция ====================
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
