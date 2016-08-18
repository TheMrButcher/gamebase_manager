#include "appcompressiondialog.h"
#include "ui_appcompressiondialog.h"
#include "settings.h"
#include <QFileDialog>

AppCompressionDialog::AppCompressionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppCompressionDialog)
{
    ui->setupUi(this);

    connect(ui->appDestinations->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onDestSelectionChanged(QItemSelection,QItemSelection)));
}

AppCompressionDialog::~AppCompressionDialog()
{
    delete ui;
}

void AppCompressionDialog::set(const App& app)
{
    ui->nameEdit->setText(app.name);

    ui->appDestinations->clear();
    const auto& settings = Settings::instance();
    foreach (auto source, settings.appSources)
        ui->appDestinations->addItem(source.path);
    ui->appDestinations->addItem(settings.downloadsDir().path);

    ui->pathEdit->setText(settings.workingDir().path);
}

QString AppCompressionDialog::name() const
{
    return ui->nameEdit->text();
}

QString AppCompressionDialog::path() const
{
    return ui->pathEdit->text();
}

void AppCompressionDialog::onDestSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    ui->selectButton->setEnabled(!ui->appDestinations->selectionModel()->selectedRows().empty());
}

void AppCompressionDialog::on_selectButton_clicked()
{
    auto rows = ui->appDestinations->selectionModel()->selectedRows();
    if (rows.empty())
        return;
    int row = rows[0].row();
    ui->pathEdit->setText(ui->appDestinations->item(row)->text());
}

void AppCompressionDialog::on_setPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути", ui->pathEdit->text());
    if (!path.isEmpty())
        ui->pathEdit->setText(path);
}
