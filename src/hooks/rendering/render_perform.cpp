#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"
#include <vector>
#include <cmath>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QWindow>

static bool is_created_window = false;

void hooks::render_perform(RenderView *this_ptr, uintptr_t mainFramebuffer, double timeJobStart) {
    hooking::get_original<&hooks::render_perform>()(this_ptr, mainFramebuffer, timeJobStart);

    // For testing window qt window creation only
    if (!is_created_window) {
        is_created_window = true;

        auto *popup = new QWidget();
        popup->setWindowTitle("Roblox Mod Loader");
        popup->resize(800, 600);
        popup->setAttribute(Qt::WA_DeleteOnClose);
        popup->setStyleSheet(
            "QWidget {"
            "   background-color: #2D2D30;"
            "   color: #FFFFFF;"
            "   font-family: 'Segoe UI', Arial, sans-serif;"
            "}"
            "QPushButton {"
            "   background-color: #0078D7;"
            "   border: none;"
            "   color: white;"
            "   padding: 8px 16px;"
            "   border-radius: 4px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #1C97EA;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0067B8;"
            "}"
            "QTabWidget::pane {"
            "   border: 1px solid #3F3F46;"
            "   background-color: #252526;"
            "   border-radius: 4px;"
            "}"
            "QTabBar::tab {"
            "   background-color: #2D2D30;"
            "   color: #FFFFFF;"
            "   padding: 8px 20px;"
            "   border-top-left-radius: 4px;"
            "   border-top-right-radius: 4px;"
            "}"
            "QTabBar::tab:selected {"
            "   background-color: #1E1E1E;"
            "   border-bottom: 2px solid #0078D7;"
            "}"
            "QTabBar::tab:hover:!selected {"
            "   background-color: #3E3E40;"
            "}"
            "QLineEdit, QTextEdit, QComboBox {"
            "   background-color: #333337;"
            "   border: 1px solid #3F3F46;"
            "   color: #FFFFFF;"
            "   padding: 6px;"
            "   border-radius: 4px;"
            "}"
            "QListWidget, QTreeWidget {"
            "   background-color: #252526;"
            "   border: 1px solid #3F3F46;"
            "   color: #FFFFFF;"
            "   border-radius: 4px;"
            "}"
            "QListWidget::item:selected, QTreeWidget::item:selected {"
            "   background-color: #0078D7;"
            "}"
            "QListWidget::item:hover, QTreeWidget::item:hover {"
            "   background-color: #3E3E40;"
            "}"
            "QCheckBox {"
            "   color: #FFFFFF;"
            "}"
            "QCheckBox::indicator {"
            "   width: 16px;"
            "   height: 16px;"
            "   border-radius: 2px;"
            "   border: 1px solid #76767A;"
            "}"
            "QCheckBox::indicator:checked {"
            "   background-color: #0078D7;"
            "}"
            "QLabel {"
            "   color: #FFFFFF;"
            "}"
            "QGroupBox {"
            "   border: 1px solid #3F3F46;"
            "   border-radius: 4px;"
            "   margin-top: 10px;"
            "   padding-top: 10px;"
            "   color: #FFFFFF;"
            "}"
        );
        auto *mainLayout = new QVBoxLayout(popup);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(15);

        auto *headerLayout = new QHBoxLayout();
        auto *logoLabel = new QLabel("Roblox Mod Loader");
        QFont logoFont = logoLabel->font();
        logoFont.setPointSize(16);
        logoFont.setBold(true);
        logoLabel->setFont(logoFont);
        logoLabel->setStyleSheet("color: #0078D7;");

        auto *versionLabel = new QLabel("v0.1.0");
        versionLabel->setStyleSheet("color: #76767A;");
        headerLayout->addWidget(logoLabel);
        headerLayout->addStretch(1);
        headerLayout->addWidget(versionLabel);
        mainLayout->addLayout(headerLayout);

        auto *tabWidget = new QTabWidget(popup);
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setDocumentMode(true);

        auto *modsTab = new QWidget();
        auto *modsLayout = new QVBoxLayout(modsTab);

        auto *modList = new QListWidget();
        modList->setAlternatingRowColors(true);
        modList->setStyleSheet(
            "QListWidget::item:alternate {"
            "   background-color: #2A2A2D;"
            "}"
        );

        QListWidgetItem *item1 = new QListWidgetItem("Example Mod 1");
        item1->setCheckState(Qt::Checked);
        modList->addItem(item1);

        QListWidgetItem *item2 = new QListWidgetItem("Example Mod 2");
        item2->setCheckState(Qt::Checked);
        modList->addItem(item2);

        QListWidgetItem *item3 = new QListWidgetItem("Example Mod 3");
        item3->setCheckState(Qt::Unchecked);
        modList->addItem(item3);

        auto *searchLayout = new QHBoxLayout();
        auto *searchIcon = new QLabel("🔍");
        auto *searchInput = new QLineEdit();
        searchInput->setPlaceholderText("Procurar mods...");
        searchLayout->addWidget(searchIcon);
        searchLayout->addWidget(searchInput);

        auto *modInfoGroupBox = new QGroupBox("Informações do Mod");
        auto *modInfoLayout = new QVBoxLayout(modInfoGroupBox);

        auto *modNameLabel = new QLabel("<b>Nome:</b> Example Mod 1");
        auto *modAuthorLabel = new QLabel("<b>Autor:</b> Revolution");
        auto *modVersionLabel = new QLabel("<b>Versão:</b> 1.0.0");
        auto *modDescriptionLabel = new QLabel(
            "<b>Descrição:</b> Este é um mod de exemplo que demonstra as funcionalidades do Roblox Mod Loader.");
        modDescriptionLabel->setWordWrap(true);

        modInfoLayout->addWidget(modNameLabel);
        modInfoLayout->addWidget(modAuthorLabel);
        modInfoLayout->addWidget(modVersionLabel);
        modInfoLayout->addWidget(modDescriptionLabel);

        auto *modActionsLayout = new QHBoxLayout();
        auto *enableButton = new QPushButton("Habilitar");
        auto *disableButton = new QPushButton("Desabilitar");
        auto *configButton = new QPushButton("Configurar");

        modActionsLayout->addWidget(enableButton);
        modActionsLayout->addWidget(disableButton);
        modActionsLayout->addWidget(configButton);

        QObject::connect(modList, &QListWidget::itemClicked, [modNameLabel, modList](QListWidgetItem *item) {
            modNameLabel->setText("<b>Nome:</b> " + item->text());
        });

        QObject::connect(enableButton, &QPushButton::clicked, [modList]() {
            if (modList->currentItem()) {
                modList->currentItem()->setCheckState(Qt::Checked);
            }
        });

        QObject::connect(disableButton, &QPushButton::clicked, [modList]() {
            if (modList->currentItem()) {
                modList->currentItem()->setCheckState(Qt::Unchecked);
            }
        });

        modsLayout->addLayout(searchLayout);
        modsLayout->addWidget(modList);
        modsLayout->addWidget(modInfoGroupBox);
        modsLayout->addLayout(modActionsLayout);

        auto *settingsTab = new QWidget();
        auto *settingsLayout = new QVBoxLayout(settingsTab);

        auto *generalSettingsGroup = new QGroupBox("Configurações Gerais");
        auto *generalSettingsLayout = new QFormLayout(generalSettingsGroup);

        auto *startupCheckbox = new QCheckBox("Carregar mods automaticamente na inicialização");
        startupCheckbox->setChecked(true);

        auto *notificationsCheckbox = new QCheckBox("Mostrar notificações");
        notificationsCheckbox->setChecked(true);

        auto *updatesCheckbox = new QCheckBox("Verificar atualizações automaticamente");
        updatesCheckbox->setChecked(true);

        auto *themeLabel = new QLabel("Tema:");
        auto *themeComboBox = new QComboBox();
        themeComboBox->addItem("Escuro (Padrão)");
        themeComboBox->addItem("Claro");
        themeComboBox->addItem("Roblox Studio");

        auto *langLabel = new QLabel("Idioma:");
        auto *langComboBox = new QComboBox();
        langComboBox->addItem("Português (Brasil)");
        langComboBox->addItem("English");
        langComboBox->addItem("Español");

        generalSettingsLayout->addRow(startupCheckbox);
        generalSettingsLayout->addRow(notificationsCheckbox);
        generalSettingsLayout->addRow(updatesCheckbox);
        generalSettingsLayout->addRow(themeLabel, themeComboBox);
        generalSettingsLayout->addRow(langLabel, langComboBox);

        auto *directoriesGroup = new QGroupBox("Diretórios");
        auto *directoriesLayout = new QFormLayout(directoriesGroup);

        auto *modsPathLayout = new QHBoxLayout();
        auto *modsPathInput = new QLineEdit("C:\\Users\\Revolution\\AppData\\Local\\Roblox\\Mods");
        auto *modsPathButton = new QPushButton("...");
        modsPathButton->setMaximumWidth(30);
        modsPathLayout->addWidget(modsPathInput);
        modsPathLayout->addWidget(modsPathButton);

        auto *robloxPathLayout = new QHBoxLayout();
        auto *robloxPathInput = new QLineEdit(
            "C:\\Users\\Revolution\\AppData\\Local\\Roblox\\Versions\\version-8611fcf01aa4494a");
        auto *robloxPathButton = new QPushButton("...");
        robloxPathButton->setMaximumWidth(30);
        robloxPathLayout->addWidget(robloxPathInput);
        robloxPathLayout->addWidget(robloxPathButton);

        directoriesLayout->addRow("Pasta de Mods:", modsPathLayout);
        directoriesLayout->addRow("Pasta do Roblox Studio:", robloxPathLayout);

        auto *settingsButtonsLayout = new QHBoxLayout();
        settingsButtonsLayout->addStretch(1);
        auto *saveSettingsButton = new QPushButton("Salvar Configurações");
        saveSettingsButton->setMinimumWidth(150);
        auto *resetSettingsButton = new QPushButton("Redefinir");
        resetSettingsButton->setMinimumWidth(100);
        settingsButtonsLayout->addWidget(resetSettingsButton);
        settingsButtonsLayout->addWidget(saveSettingsButton);

        settingsLayout->addWidget(generalSettingsGroup);
        settingsLayout->addWidget(directoriesGroup);
        settingsLayout->addStretch(1);
        settingsLayout->addLayout(settingsButtonsLayout);

        auto *aboutTab = new QWidget();
        auto *aboutLayout = new QVBoxLayout(aboutTab);

        auto *aboutTextLayout = new QVBoxLayout();
        auto *aboutTitleLabel = new QLabel("Roblox Mod Loader");
        QFont aboutTitleFont = aboutTitleLabel->font();
        aboutTitleFont.setPointSize(20);
        aboutTitleFont.setBold(true);
        aboutTitleLabel->setFont(aboutTitleFont);
        aboutTitleLabel->setAlignment(Qt::AlignCenter);

        auto *versionInfoLabel = new QLabel("Versão 0.1.0");
        versionInfoLabel->setAlignment(Qt::AlignCenter);

        auto *descriptionLabel = new QLabel("Um carregador de mods para o ROBLOX Studio que permite carregar "
            "mods dentro do Studio. Este software é de código aberto e está "
            "disponível sob os termos da licença MIT.");
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setAlignment(Qt::AlignCenter);

        auto *authorLabel = new QLabel("Desenvolvido por Revolution");
        authorLabel->setAlignment(Qt::AlignCenter);

        auto *linksLayout = new QHBoxLayout();
        linksLayout->setAlignment(Qt::AlignCenter);

        auto *githubButton = new QPushButton("GitHub");
        auto *websiteButton = new QPushButton("Website");
        auto *donateButton = new QPushButton("Doar");

        linksLayout->addWidget(githubButton);
        linksLayout->addWidget(websiteButton);
        linksLayout->addWidget(donateButton);

        aboutTextLayout->addWidget(aboutTitleLabel);
        aboutTextLayout->addWidget(versionInfoLabel);
        aboutTextLayout->addSpacing(20);
        aboutTextLayout->addWidget(descriptionLabel);
        aboutTextLayout->addSpacing(20);
        aboutTextLayout->addWidget(authorLabel);
        aboutTextLayout->addSpacing(10);
        aboutTextLayout->addLayout(linksLayout);

        aboutLayout->addLayout(aboutTextLayout);
        aboutLayout->addStretch(1);

        tabWidget->addTab(modsTab, "Mods");
        tabWidget->addTab(settingsTab, "Configurações");
        tabWidget->addTab(aboutTab, "Sobre");

        mainLayout->addWidget(tabWidget);

        auto *statusLayout = new QHBoxLayout();
        auto *statusLabel = new QLabel("3 mods carregados | Roblox Studio detectado");
        auto *refreshButton = new QPushButton("Atualizar");
        statusLayout->addWidget(statusLabel);
        statusLayout->addStretch(1);
        statusLayout->addWidget(refreshButton);
        mainLayout->addLayout(statusLayout);

        popup->setLayout(mainLayout);
        popup->show();
    }
}
