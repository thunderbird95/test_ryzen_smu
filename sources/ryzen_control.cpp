#include "ryzen_control.h"
#include "ui_ryzen_control.h"

#include <QStringList>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QScrollBar>
#include <QDateTime>

#include <math.h>

const int graphStatisticLength = 20 * 60;

//-----------------------------------------------------------------------------------------------------------------------------------
RyzenControl::RyzenControl(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RyzenControl), /*m_statisticWidget(this),*/ m_currentGfxClkIndex(0)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &RyzenControl::setData);

    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.setValueToSmu(ry,ui->isPsmu->isChecked(),ui->regIndex->value(),val); }); m_functionDescriptions.push_back("Custom reg");

    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_stapm_limit(ry,val); }); m_functionDescriptions.push_back("set_stapm_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_fast_limit(ry,val); }); m_functionDescriptions.push_back("set_fast_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_slow_limit(ry,val); }); m_functionDescriptions.push_back("set_slow_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_slow_time(ry,val); }); m_functionDescriptions.push_back("set_slow_time");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_stapm_time(ry,val); }); m_functionDescriptions.push_back("set_stapm_time");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_tctl_temp(ry,val); }); m_functionDescriptions.push_back("set_tctl_temp");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrm_current(ry,val); }); m_functionDescriptions.push_back("set_vrm_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrmsoc_current(ry,val); }); m_functionDescriptions.push_back("set_vrmsoc_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrmgfx_current(ry,val); }); m_functionDescriptions.push_back("set_vrmgfx_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrmcvip_current(ry,val); }); m_functionDescriptions.push_back("set_vrmcvip_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrmmax_current(ry,val); }); m_functionDescriptions.push_back("set_vrmmax_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrmgfxmax_current(ry,val); }); m_functionDescriptions.push_back("set_vrmgfxmax_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_vrmsocmax_current(ry,val); }); m_functionDescriptions.push_back("set_vrmsocmax_current");

    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_psi0_current(ry,val); }); m_functionDescriptions.push_back("set_psi0_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_psi3cpu_current(ry,val); }); m_functionDescriptions.push_back("set_psi3cpu_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_psi0soc_current(ry,val); }); m_functionDescriptions.push_back("set_psi0soc_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_psi3gfx_current(ry,val); }); m_functionDescriptions.push_back("set_psi3gfx_current");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_max_gfxclk_freq(ry,val); }); m_functionDescriptions.push_back("set_max_gfxclk_freq");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_min_gfxclk_freq(ry,val); }); m_functionDescriptions.push_back("set_min_gfxclk_freq");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_max_socclk_freq(ry,val); }); m_functionDescriptions.push_back("set_max_socclk_freq");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_min_socclk_freq(ry,val); }); m_functionDescriptions.push_back("set_min_socclk_freq");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_max_fclk_freq(ry,val); }); m_functionDescriptions.push_back("set_max_fclk_freq");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_min_fclk_freq(ry,val); }); m_functionDescriptions.push_back("set_min_fclk_freq");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_max_vcn(ry,val); }); m_functionDescriptions.push_back("set_max_vcn");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_min_vcn(ry,val); }); m_functionDescriptions.push_back("set_min_vcn");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_max_lclk(ry,val); }); m_functionDescriptions.push_back("set_max_lclk");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_min_lclk(ry,val); }); m_functionDescriptions.push_back("set_min_lclk");

    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_prochot_deassertion_ramp(ry,val); }); m_functionDescriptions.push_back("set_prochot_deassertion_ramp");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_apu_skin_temp_limit(ry,val); }); m_functionDescriptions.push_back("set_apu_skin_temp_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_dgpu_skin_temp_limit(ry,val); }); m_functionDescriptions.push_back("set_dgpu_skin_temp_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_apu_slow_limit(ry,val); }); m_functionDescriptions.push_back("set_apu_slow_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_skin_temp_power_limit(ry,val); }); m_functionDescriptions.push_back("set_skin_temp_power_limit");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_gfx_clk(ry,val); }); m_functionDescriptions.push_back("set_gfx_clk");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_oc_clk(ry,val); }); m_functionDescriptions.push_back("set_oc_clk");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_per_core_oc_clk(ry,val); }); m_functionDescriptions.push_back("set_per_core_oc_clk");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_oc_volt(ry,val); }); m_functionDescriptions.push_back("set_oc_volt");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_disable_oc(ry); }); m_functionDescriptions.push_back("set_disable_oc");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_enable_oc(ry); }); m_functionDescriptions.push_back("set_enable_oc");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_power_saving(ry); }); m_functionDescriptions.push_back("set_power_saving");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_max_performance(ry); }); m_functionDescriptions.push_back("set_max_performance");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_coall(ry,val); }); m_functionDescriptions.push_back("set_coall");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_coper(ry,val); }); m_functionDescriptions.push_back("set_coper");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_cogfx(ry,val); }); m_functionDescriptions.push_back("set_gfx_curve_offset");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_gfx_clk_auto(ry); }); m_functionDescriptions.push_back("set_gfx_clk_auto");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_fclk_and_memclk(ry,val); }); m_functionDescriptions.push_back("set_fclk_and_memclk");
    m_functions.push_back([this] (ryzen_access ry,int val) { return m_apuDriver.set_fclk_and_memclk_auto(ry); }); m_functionDescriptions.push_back("set_fclk_and_memclk_auto");

    connect(ui->comboBox, &QComboBox::currentTextChanged, [ this ] (const QString&) {
        bool isCustom = (ui->comboBox->currentIndex() == 0);
        ui->regIndex->setEnabled(isCustom);
        ui->isPsmu->setEnabled(isCustom);
        ui->value1->setEnabled(isCustom);    } );

    connect(ui->isValue0hex, &QCheckBox::toggled, [this] (bool isHex) {
        ui->spinBox->setDisplayIntegerBase(isHex ? 16 : 10);
        ui->spinBox->setPrefix(isHex ? QString("0x") : QString(""));
    } );

    for (int i = 0; i < m_functions.size(); i++)
        ui->comboBox->addItem(QString("#%1: %2").arg(i).arg(QString(m_functionDescriptions[i])));

    //init RyzenAdj and validate that it was able to
    m_ry = m_apuDriver.init_ryzenadj();
    if(!m_ry){
        delete ui;
        close();
        return;// -1;
    }

    //shows info header before init_table
    if (1) {
        QStringList familyNames;
        familyNames << "Raven" << "Picasso" << "Renoir" << "Cezanne" << "Dali" << "Lucienne" << "Vangogh" << "Rembrandt";

        //qDebug("CPU Family: %s\n", familyNames[(int)apuDriver.get_cpu_family(ry)].toLatin1());
        qDebug("SMU BIOS Interface Version: %d\n", m_apuDriver.get_bios_if_ver(m_ry));
        //printf("Version: v" STRINGIFY(RYZENADJ_REVISION_VER) "." STRINGIFY(RYZENADJ_MAJOR_VER) "." STRINGIFY(RYZENADJ_MINIOR_VER) " \n");
        ui->label->setText(QString("Family: %1, BIOS: %2, table version %3, size %4").arg(familyNames[(int)m_apuDriver.get_cpu_family(m_ry)]).
                arg(m_apuDriver.get_bios_if_ver(m_ry)).arg(m_apuDriver.get_table_ver(m_ry)).arg(m_apuDriver.get_table_size(m_ry)));

        m_lastTableValues.resize(m_apuDriver.get_table_size(m_ry));
        m_lastTableValues.fill(NAN);
        m_tableStatisticData.resize(STATISTIC_MEASURES_COUNT);
        m_tableStatisticData.fill(QVector<float>(m_apuDriver.get_table_size(m_ry), NAN));
        m_tableStatisticMin.resize(m_apuDriver.get_table_size(m_ry));
        m_tableStatisticMax.resize(m_apuDriver.get_table_size(m_ry));
        m_tableStatisticMid.resize(m_apuDriver.get_table_size(m_ry));

        m_tableDescriptions.resize(m_apuDriver.get_table_size(m_ry) / 4);
        m_tableDescriptions.fill("UNKNOWN");

        if (m_apuDriver.get_table_ver(m_ry) == 0x370005)
        {
            m_tableDescriptions[0] = "stapm_limit";
            m_tableDescriptions[1] = "stapm_value";
            m_tableDescriptions[2] = "fast_limit";
            m_tableDescriptions[3] = "fast_value";
            m_tableDescriptions[4] = "slow_limit";
            m_tableDescriptions[5] = "slow_value";

            m_tableDescriptions[6] = "apu_slow_limit";
            m_tableDescriptions[7] = "apu_slow_value";

            m_tableDescriptions[8] = "vrm_current";
            m_tableDescriptions[9] = "vrm_current_value";
            m_tableDescriptions[10] = "vrmsoc_current";
            m_tableDescriptions[11] = "vrmsoc_current_value";
            m_tableDescriptions[12] = "vrmmax_current";
            m_tableDescriptions[13] = "vrmmax_current_value";
            m_tableDescriptions[14] = "vrmsocmax_current";
            m_tableDescriptions[15] = "vrmsocmax_current_value";

            m_tableDescriptions[16] = "tctl_temp";
            m_tableDescriptions[17] = "tctl_temp_value";

            m_tableDescriptions[22] = "apu_skin_temp_limit";
            m_tableDescriptions[23] = "apu_skin_temp_value";
            m_tableDescriptions[24] = "dgpu_skin_temp_limit";
            m_tableDescriptions[25] = "dgpu_skin_temp_value";

            m_tableDescriptions[30] = "psi0_current";
            m_tableDescriptions[32] = "psi0soc_current";

            m_tableDescriptions[63] = "cclk_setpoint";
            m_tableDescriptions[64] = "cclk_busy_value";

            m_tableDescriptions[551] = "stapm_time";
            m_tableDescriptions[552] = "slow_time";

            for (int i = 0; i < 8; i++)
                m_tableDescriptions[199 + i] = QString("core_power_%1").arg(i);
            for (int i = 0; i < 8; i++)
                m_tableDescriptions[207 + i] = QString("core_volt_%1").arg(i);
            for (int i = 0; i < 8; i++)
                m_tableDescriptions[215 + i] = QString("core_temp_%1").arg(i);
            for (int i = 0; i < 8; i++)
                m_tableDescriptions[239 + i] = QString("core_clk_%1").arg(i);

            m_tableDescriptions[353] = "l3_clk";
            m_tableDescriptions[343] = "l3_logic";
            m_tableDescriptions[345] = "l3_vddm";
            m_tableDescriptions[347] = "l3_temp";

            m_tableDescriptions[372] = "gfx_clk";
            m_tableDescriptions[369] = "gfx_volt";
            m_tableDescriptions[370] = "gfx_temp";

            m_tableDescriptions[378] = "fclk";
            m_tableDescriptions[380] = "mem_clk";

            m_tableDescriptions[102] = "soc_volt";
            m_tableDescriptions[104] = "soc_power";
            m_tableDescriptions[38] = "socket_power";
        }
        else if (m_apuDriver.get_table_ver(m_ry) == 0x400005)
        {
            m_tableDescriptions[0] = "stapm_limit";
            m_tableDescriptions[1] = "stapm_value";
            m_tableDescriptions[2] = "fast_limit";
            m_tableDescriptions[3] = "fast_value";
            m_tableDescriptions[4] = "slow_limit";
            m_tableDescriptions[5] = "slow_value";

            m_tableDescriptions[6] = "apu_slow_limit";
            m_tableDescriptions[7] = "apu_slow_value";

            m_tableDescriptions[8] = "vrm_current";
            m_tableDescriptions[9] = "vrm_current_value";
            m_tableDescriptions[10] = "vrmsoc_current";
            m_tableDescriptions[11] = "vrmsoc_current_value";
            m_tableDescriptions[12] = "vrmmax_current";
            m_tableDescriptions[13] = "vrmmax_current_value";
            m_tableDescriptions[14] = "vrmsocmax_current";
            m_tableDescriptions[15] = "vrmsocmax_current_value";

            m_tableDescriptions[16] = "tctl_temp";
            m_tableDescriptions[17] = "tctl_temp_value";

            m_tableDescriptions[22] = "apu_skin_temp_limit";
            m_tableDescriptions[23] = "apu_skin_temp_value";
            m_tableDescriptions[24] = "dgpu_skin_temp_limit";
            m_tableDescriptions[25] = "dgpu_skin_temp_value";

            m_tableDescriptions[30] = "psi0_current";
            m_tableDescriptions[32] = "psi0soc_current";

            m_tableDescriptions[64] = "cclk_setpoint";
            m_tableDescriptions[65] = "cclk_busy_value";

            m_tableDescriptions[582] = "stapm_time";
            m_tableDescriptions[583] = "slow_time";

            for (int i = 0; i < 8; i++)
                m_tableDescriptions[200 + i] = QString("core_power_%1").arg(i);
            for (int i = 0; i < 8; i++)
                m_tableDescriptions[208 + i] = QString("core_volt_%1").arg(i);
            for (int i = 0; i < 8; i++)
                m_tableDescriptions[216 + i] = QString("core_temp_%1").arg(i);
            for (int i = 0; i < 8; i++)
                m_tableDescriptions[240 + i] = QString("core_clk_%1").arg(i);

            m_tableDescriptions[389] = "l3_clk";
            m_tableDescriptions[384] = "l3_logic";
            m_tableDescriptions[385] = "l3_vddm";
            m_tableDescriptions[386] = "l3_temp";

            m_tableDescriptions[402] = "gfx_clk";
            m_tableDescriptions[399] = "gfx_volt";
            m_tableDescriptions[400] = "gfx_temp";

            m_tableDescriptions[409] = "fclk";
            m_tableDescriptions[411] = "mem_clk";

            m_tableDescriptions[103] = "soc_volt";
            m_tableDescriptions[105] = "soc_power";
            m_tableDescriptions[38] = "socket_power";
        }

        int ryzenTableLength = m_apuDriver.get_table_size(m_ry) / sizeof(float);
        m_currentIndex = 0;
        m_tableValues = QVector<QVector<float>>(graphStatisticLength, QVector<float>(ryzenTableLength, 0.0));
        m_tableReadTimes = QVector<int>(graphStatisticLength, QTime::currentTime().msecsSinceStartOfDay() / 1000);
        m_graphsWidget.setDescriptions(m_tableDescriptions);
        m_statisticWidget.setDescriptions(m_tableDescriptions);

        int errorcode = m_apuDriver.refresh_table(m_ry);
        if(errorcode){
            printf("Unable to refresh power metric table: %d\n", errorcode);
            return;
        }

        QStringList info;
        info << QString( "STAPM LIMIT %1").arg(m_apuDriver.get_stapm_limit(m_ry));
        info << QString("STAPM VALUE %1").arg(m_apuDriver.get_stapm_value(m_ry));
        info << QString("PPT LIMIT FAST %1").arg(m_apuDriver.get_fast_limit(m_ry));
        info << QString("PPT VALUE FAST %1").arg(m_apuDriver.get_fast_value(m_ry));
        info << QString("PPT LIMIT SLOW %1").arg(m_apuDriver.get_slow_limit(m_ry));
        info << QString("PPT VALUE SLOW %1").arg(m_apuDriver.get_slow_value(m_ry));
        info << QString("StapmTimeConst %1").arg(m_apuDriver.get_stapm_time(m_ry));
        info << QString("SlowPPTTimeConst %1").arg(m_apuDriver.get_slow_time(m_ry));
        info << QString("PPT LIMIT APU %1").arg(m_apuDriver.get_apu_slow_limit(m_ry));
        info << QString("PPT VALUE APU %1").arg(m_apuDriver.get_apu_slow_value(m_ry));
        info << QString("TDC LIMIT VDD %1").arg(m_apuDriver.get_vrm_current(m_ry));
        info << QString("TDC VALUE VDD %1").arg(m_apuDriver.get_vrm_current_value(m_ry));
        info << QString("TDC LIMIT SOC %1").arg(m_apuDriver.get_vrmsoc_current(m_ry));
        info << QString("TDC VALUE SOC %1").arg(m_apuDriver.get_vrmsoc_current_value(m_ry));
        info << QString("EDC LIMIT VDD %1").arg(m_apuDriver.get_vrmmax_current(m_ry));
        info << QString("EDC VALUE VDD %1").arg(m_apuDriver.get_vrmmax_current_value(m_ry));
        info << QString("EDC LIMIT SOC %1").arg(m_apuDriver.get_vrmsocmax_current(m_ry));
        info << QString("EDC VALUE SOC %1").arg(m_apuDriver.get_vrmsocmax_current_value(m_ry));
        info << QString("THM LIMIT CORE %1").arg(m_apuDriver.get_tctl_temp(m_ry));
        info << QString("THM VALUE CORE %1").arg(m_apuDriver.get_tctl_temp_value(m_ry));
        info << QString("STT LIMIT APU %1").arg(m_apuDriver.get_apu_skin_temp_limit(m_ry));
        info << QString("STT VALUE APU %1").arg(m_apuDriver.get_apu_skin_temp_value(m_ry));
        info << QString("STT LIMIT dGPU %1").arg(m_apuDriver.get_dgpu_skin_temp_limit(m_ry));
        info << QString("STT VALUE dGPU %1").arg(m_apuDriver.get_dgpu_skin_temp_value(m_ry));
        info << QString("CCLK Boost SETPOINT %1").arg(m_apuDriver.get_cclk_setpoint(m_ry));
        info << QString("CCLK BUSY VALUE %1").arg(m_apuDriver.get_cclk_busy_value(m_ry));

        ui->plainTextEdit->setPlainText(info.join("\n"));
    }

    connect(ui->enableRefleshTable, &QCheckBox::toggled, [ this ] (bool isChecked) { if (isChecked) m_periodicTimer.start(); else m_periodicTimer.stop(); } );

    m_periodicTimer.setInterval(1000);
    m_periodicTimer.setSingleShot(false);
    connect(&m_periodicTimer, &QTimer::timeout, this, &RyzenControl::updateData);
    if (ui->enableRefleshTable->isChecked())
        m_periodicTimer.start();

    connect(ui->gfxClkAutoset, &QCheckBox::toggled,  [ this ] (bool isChecked) {
        if (!isChecked) { return; }
        if (!ui->enableRefleshTable->isChecked()) ui->enableRefleshTable->setChecked(true);
        m_apuDriver.set_fclk_and_memclk(m_ry, 0);
        m_apuDriver.set_power_saving(m_ry);
        m_apuDriver.set_cogfx(m_ry, -1000);
        m_apuDriver.set_coall(m_ry, -1000);
        m_currentGfxClk = 0;//MIN_GFX_CLK;
        m_currentGfxClkIndex = 0;
        m_apuDriver.set_gfx_clk_auto(m_ry); } );

    connect(ui->openGraphsWidget, &QPushButton::clicked, &m_graphsWidget, &Graphs::show);
    connect(this, &RyzenControl::displayData, &m_graphsWidget, &Graphs::displayGraph);

    connect(ui->openStatisticWidget, &QPushButton::clicked, &m_statisticWidget, &Statistic::show);
    connect(this, &RyzenControl::currentValuesUpdate, &m_statisticWidget, &Statistic::handleNewValues);

    m_statisticWidget.setRyzenControlLink(this);
}

//-----------------------------------------------------------------------------------------------------------------------------------
RyzenControl::~RyzenControl()
{
    m_apuDriver.cleanup_ryzenadj(m_ry);
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void RyzenControl::closeEvent(QCloseEvent *event)
{
    m_graphsWidget.close();
    m_statisticWidget.close();
    event->accept();
}

//-----------------------------------------------------------------------------------------------------------------------------------
QString RyzenControl::setRegValue(bool isPsmu, uint32_t address, uint32_t value0, uint32_t value1, uint32_t value2)
{
    m_periodicTimer.stop();
    int result = m_apuDriver.setValueToSmu(m_ry, isPsmu, address, value0, value1, value2);
    m_periodicTimer.start();
    QString resultDescription = QString();
    switch (result)
    {
        case 0:
            // ALL OK
            break;
        case ADJ_ERR_SMU_REJECTED:
            resultDescription = QString("COMMAND REJECTED");
            break;
        case ADJ_ERR_SMU_UNSUPPORTED:
            resultDescription = QString("COMMAND UNSUPPORTED");
            break;
        default:
            resultDescription = QString("UNKNOWN RESULT (%1)").arg(result);
    }
    return resultDescription;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void RyzenControl::updateData()
{
    int errorcode = m_apuDriver.refresh_table(m_ry);
    if(errorcode){
        ui->plainTextEdit->setPlainText("Error");
        return;
    }

    QString gfxClockInfo;
    if (ui->gfxClkAutoset->isChecked())
    {
        float currentBusyValue = m_apuDriver.get_table_values(m_ry)[GFX_BUSY_REG];
        int32_t nextGfxClk = m_currentGfxClk;
#if 0
        if ((currentBusyValue > MAX_GFX_BUSY_VALUE) && (m_currentGfxClk < MAX_GFX_CLK))
            nextGfxClk = m_currentGfxClk + GFX_CLK_STEP;
        else if ((currentBusyValue < MIN_GFX_BUSY_VALUE) && (m_currentGfxClk > MIN_GFX_CLK))
            nextGfxClk = m_currentGfxClk - GFX_CLK_STEP;
        if (nextGfxClk != m_currentGfxClk)
        {
            if (nextGfxClk <= 1500)
                m_apuDriver.set_gfx_clk_auto(m_ry);
            else
                m_apuDriver.set_gfx_clk(m_ry, nextGfxClk);
            m_currentGfxClk = nextGfxClk;
        }
#else
        float currentCpuBusyValue = m_apuDriver.get_table_values(m_ry)[CCLK_BUSY_REG];
        int32_t nextGfxClkIndex = m_currentGfxClkIndex;

        if (m_gfxClkSetTimeout)
            m_gfxClkSetTimeout--;
        else
        {
            if (m_currentGfxClkIndex < 0)                       m_currentGfxClkIndex = 0;
            if (m_currentGfxClkIndex > (GFX_CLOCKS_COUNT - 1))  m_currentGfxClkIndex = (GFX_CLOCKS_COUNT - 1);

            const float minGfxBusyValues[GFX_CLOCKS_COUNT] = MIN_GFX_BUSY_VALUES;
            const float maxCclkBusyValues[GFX_CLOCKS_COUNT] = MAX_CCLK_BUSY_VALUES;
            const float maxGfxBusyValues[GFX_CLOCKS_COUNT] = MAX_GFX_BUSY_VALUES;
            const float minCclkBusyValues[GFX_CLOCKS_COUNT] = MIN_CCLK_BUSY_VALUES;
            const int32_t avaluableGfxFrequencies[GFX_CLOCKS_COUNT] = GFX_FREQUENCIES_LIST;

            if (((currentBusyValue < minGfxBusyValues[m_currentGfxClkIndex]) || (currentCpuBusyValue > maxCclkBusyValues[m_currentGfxClkIndex])) && (m_currentGfxClkIndex > 0))
                nextGfxClkIndex--;
            else if ((currentBusyValue > maxGfxBusyValues[m_currentGfxClkIndex]) && (currentCpuBusyValue < minCclkBusyValues[m_currentGfxClkIndex]) && (m_currentGfxClkIndex < (GFX_CLOCKS_COUNT - 1)))
                nextGfxClkIndex++;

            if (nextGfxClkIndex != m_currentGfxClkIndex)
            {
                nextGfxClk = avaluableGfxFrequencies[nextGfxClkIndex];
                if (nextGfxClk <= 1500)
                    m_apuDriver.set_gfx_clk_auto(m_ry);
                else
                    m_apuDriver.set_gfx_clk(m_ry, nextGfxClk);
                m_currentGfxClkIndex = nextGfxClkIndex;
                m_gfxClkSetTimeout = GFX_CLK_SET_TIMEOUT;
            }
        }
#endif
        gfxClockInfo = QString("Vega busy value %1%, last clk %2, next - %3").arg(currentBusyValue).arg(m_currentGfxClk).arg(nextGfxClk);
        m_currentGfxClk = nextGfxClk;
    }

#ifndef DEBUG_MODE
    QStringList info;

    info << QString( "STAPM LIMIT %1").arg(m_apuDriver.get_stapm_limit(m_ry));
    info << QString("STAPM VALUE %1").arg(m_apuDriver.get_stapm_value(m_ry));
    info << QString("PPT LIMIT FAST %1").arg(m_apuDriver.get_fast_limit(m_ry));
    info << QString("PPT VALUE FAST %1").arg(m_apuDriver.get_fast_value(m_ry));
    info << QString("PPT LIMIT SLOW %1").arg(m_apuDriver.get_slow_limit(m_ry));
    info << QString("PPT VALUE SLOW %1").arg(m_apuDriver.get_slow_value(m_ry));
    info << QString("StapmTimeConst %1").arg(m_apuDriver.get_stapm_time(m_ry));
    info << QString("SlowPPTTimeConst %1").arg(m_apuDriver.get_slow_time(m_ry));
    info << QString("PPT LIMIT APU %1").arg(m_apuDriver.get_apu_slow_limit(m_ry));
    info << QString("PPT VALUE APU %1").arg(m_apuDriver.get_apu_slow_value(m_ry));
    info << QString("TDC LIMIT VDD %1").arg(m_apuDriver.get_vrm_current(m_ry));
    info << QString("TDC VALUE VDD %1").arg(m_apuDriver.get_vrm_current_value(m_ry));
    info << QString("TDC LIMIT SOC %1").arg(m_apuDriver.get_vrmsoc_current(m_ry));
    info << QString("TDC VALUE SOC %1").arg(m_apuDriver.get_vrmsoc_current_value(m_ry));
    info << QString("EDC LIMIT VDD %1").arg(m_apuDriver.get_vrmmax_current(m_ry));
    info << QString("EDC VALUE VDD %1").arg(m_apuDriver.get_vrmmax_current_value(m_ry));
    info << QString("EDC LIMIT SOC %1").arg(m_apuDriver.get_vrmsocmax_current(m_ry));
    info << QString("EDC VALUE SOC %1").arg(m_apuDriver.get_vrmsocmax_current_value(m_ry));
    info << QString("THM LIMIT CORE %1").arg(m_apuDriver.get_tctl_temp(m_ry));
    info << QString("THM VALUE CORE %1").arg(m_apuDriver.get_tctl_temp_value(m_ry));
    info << QString("STT LIMIT APU %1").arg(m_apuDriver.get_apu_skin_temp_limit(m_ry));
    info << QString("STT VALUE APU %1").arg(m_apuDriver.get_apu_skin_temp_value(m_ry));
    info << QString("STT LIMIT dGPU %1").arg(m_apuDriver.get_dgpu_skin_temp_limit(m_ry));
    info << QString("STT VALUE dGPU %1").arg(m_apuDriver.get_dgpu_skin_temp_value(m_ry));
    info << QString("CCLK Boost SETPOINT %1").arg(m_apuDriver.get_cclk_setpoint(m_ry));
    info << QString("CCLK BUSY VALUE %1").arg(m_apuDriver.get_cclk_busy_value(m_ry));

    info << QString("");

    for (int i = 0; i < CORES_COUNT; i++)
    {
        if (m_apuDriver.get_core_volt(m_ry,i) < 0.00000001)
            continue;
        info << QString("Core %1, clk %2, volt %3, power %4, temp %5").arg(i).arg(m_apuDriver.get_core_clk(m_ry,i)).arg(m_apuDriver.get_core_volt(m_ry,i)).
                arg(m_apuDriver.get_core_power(m_ry,i)).arg(m_apuDriver.get_core_temp(m_ry,i));
    }
    info << QString("L3 clk %1, logic %2, volt %3, temp %4").arg(m_apuDriver.get_l3_clk(m_ry)).arg(m_apuDriver.get_l3_logic(m_ry)).
            arg(m_apuDriver.get_l3_vddm(m_ry)).arg(m_apuDriver.get_l3_temp(m_ry));
    info << QString("VEGA clk %1, volt %2, temp %3").arg(m_apuDriver.get_gfx_clk(m_ry)).arg(m_apuDriver.get_gfx_volt(m_ry)).arg(m_apuDriver.get_gfx_temp(m_ry));
    info << QString("Memclk %1, fclk %2").arg(m_apuDriver.get_mem_clk(m_ry)).arg(m_apuDriver.get_fclk(m_ry));
    info << QString("SocPower %1, SocVolt %2, SocketPower %3").arg(m_apuDriver.get_soc_power(m_ry)).arg(m_apuDriver.get_soc_volt(m_ry)).arg(m_apuDriver.get_socket_power(m_ry));

    ui->plainTextEdit->setPlainText(info. join("\n"));
#else
    int ryzenTableLength = m_apuDriver.get_table_size(m_ry) / sizeof(float);
    for (int i = 0; i < ryzenTableLength; i++)
        m_tableValues[m_currentIndex][i] = m_apuDriver.get_table_values(m_ry)[i];
    m_tableReadTimes[m_currentIndex] = QTime::currentTime().msecsSinceStartOfDay() / 1000;
    emit currentValuesUpdate(m_tableValues[m_currentIndex]);
    m_currentIndex = (m_currentIndex + 1) % graphStatisticLength;
    emit displayData(m_tableValues, m_tableReadTimes, m_tableDescriptions, m_currentIndex);

    if (ui->enableExtendedInfo->isChecked())
    {
        int lastScrollBarValue = -1;
        if (ui->plainTextEdit->verticalScrollBar())
            lastScrollBarValue = ui->plainTextEdit->verticalScrollBar()->value();

        QStringList info;
        if (!gfxClockInfo.isEmpty())
            info << gfxClockInfo;
        for (int i = 0; i < m_tableDescriptions.length(); i++)
            m_lastTableValues[i] = m_apuDriver.get_table_values(m_ry)[i];
        for (int i = 0; i < m_tableDescriptions.length(); i++)
            info << QString("#%1. %2: %3").arg(i).arg(m_tableDescriptions[i]).arg(m_apuDriver.get_table_values(m_ry)[i]);
        ui->plainTextEdit->setPlainText(info.join("\n"));

        if (lastScrollBarValue != -1)
            ui->plainTextEdit->verticalScrollBar()->setValue(lastScrollBarValue);
    }
    else
    {
        QStringList info;        

        if (!gfxClockInfo.isEmpty())
            info << gfxClockInfo;

        info << QString( "STAPM LIMIT %1").arg(m_apuDriver.get_stapm_limit(m_ry));
        info << QString("STAPM VALUE %1").arg(m_apuDriver.get_stapm_value(m_ry));
        info << QString("PPT LIMIT FAST %1").arg(m_apuDriver.get_fast_limit(m_ry));
        info << QString("PPT VALUE FAST %1").arg(m_apuDriver.get_fast_value(m_ry));
        info << QString("PPT LIMIT SLOW %1").arg(m_apuDriver.get_slow_limit(m_ry));
        info << QString("PPT VALUE SLOW %1").arg(m_apuDriver.get_slow_value(m_ry));
        info << QString("StapmTimeConst %1").arg(m_apuDriver.get_stapm_time(m_ry));
        info << QString("SlowPPTTimeConst %1").arg(m_apuDriver.get_slow_time(m_ry));
        info << QString("PPT LIMIT APU %1").arg(m_apuDriver.get_apu_slow_limit(m_ry));
        info << QString("PPT VALUE APU %1").arg(m_apuDriver.get_apu_slow_value(m_ry));
        info << QString("TDC LIMIT VDD %1").arg(m_apuDriver.get_vrm_current(m_ry));
        info << QString("TDC VALUE VDD %1").arg(m_apuDriver.get_vrm_current_value(m_ry));
        info << QString("TDC LIMIT SOC %1").arg(m_apuDriver.get_vrmsoc_current(m_ry));
        info << QString("TDC VALUE SOC %1").arg(m_apuDriver.get_vrmsoc_current_value(m_ry));
        info << QString("EDC LIMIT VDD %1").arg(m_apuDriver.get_vrmmax_current(m_ry));
        info << QString("EDC VALUE VDD %1").arg(m_apuDriver.get_vrmmax_current_value(m_ry));
        info << QString("EDC LIMIT SOC %1").arg(m_apuDriver.get_vrmsocmax_current(m_ry));
        info << QString("EDC VALUE SOC %1").arg(m_apuDriver.get_vrmsocmax_current_value(m_ry));
        info << QString("THM LIMIT CORE %1").arg(m_apuDriver.get_tctl_temp(m_ry));
        info << QString("THM VALUE CORE %1").arg(m_apuDriver.get_tctl_temp_value(m_ry));
        info << QString("STT LIMIT APU %1").arg(m_apuDriver.get_apu_skin_temp_limit(m_ry));
        info << QString("STT VALUE APU %1").arg(m_apuDriver.get_apu_skin_temp_value(m_ry));
        info << QString("STT LIMIT dGPU %1").arg(m_apuDriver.get_dgpu_skin_temp_limit(m_ry));
        info << QString("STT VALUE dGPU %1").arg(m_apuDriver.get_dgpu_skin_temp_value(m_ry));
        info << QString("CCLK Boost SETPOINT %1").arg(m_apuDriver.get_cclk_setpoint(m_ry));
        info << QString("CCLK BUSY VALUE %1").arg(m_apuDriver.get_cclk_busy_value(m_ry));

        info << QString("");

        for (int i = 0; i < CORES_COUNT; i++)
        {
            if (m_apuDriver.get_core_volt(m_ry,i) < 0.00000001)
                continue;
            info << QString("Core %1, clk %2, volt %3, power %4, temp %5").arg(i).arg(m_apuDriver.get_core_clk(m_ry,i)).arg(m_apuDriver.get_core_volt(m_ry,i)).
                    arg(m_apuDriver.get_core_power(m_ry,i)).arg(m_apuDriver.get_core_temp(m_ry,i));
        }
        info << QString("L3 clk %1, logic %2, volt %3, temp %4").arg(m_apuDriver.get_l3_clk(m_ry)).arg(m_apuDriver.get_l3_logic(m_ry)).
                arg(m_apuDriver.get_l3_vddm(m_ry)).arg(m_apuDriver.get_l3_temp(m_ry));
        info << QString("VEGA clk %1, volt %2, temp %3").arg(m_apuDriver.get_gfx_clk(m_ry)).arg(m_apuDriver.get_gfx_volt(m_ry)).arg(m_apuDriver.get_gfx_temp(m_ry));
        info << QString("Memclk %1, fclk %2").arg(m_apuDriver.get_mem_clk(m_ry)).arg(m_apuDriver.get_fclk(m_ry));
        info << QString("SocPower %1, SocVolt %2, SocketPower %3").arg(m_apuDriver.get_soc_power(m_ry)).arg(m_apuDriver.get_soc_volt(m_ry)).arg(m_apuDriver.get_socket_power(m_ry));

        ui->plainTextEdit->setPlainText(info.join("\n"));
    }
#endif
}

void RyzenControl::setData()
{
    m_periodicTimer.stop();

    int index = ui->comboBox->currentIndex();
    int value = ui->spinBox->value();
    int result;

#ifdef  DEBUG_MODE
    if ((index == 0) && (ui->enableExtendedInfo->isChecked()))
    {
        for (int i = 0; i < STATISTIC_MEASURES_COUNT; i++)
        {
            if (i != 0)
                QThread::msleep(STATISTIC_MEASURES_INTERVAL);
            int errorcode = m_apuDriver.refresh_table(m_ry);
            if (errorcode)
            {
                QMessageBox::warning(this, QString("Table reflesh error before execution"), QString("Table reflesh error before execution %1, attemp count %2").
                                     arg(errorcode).arg(i));
                return;
            }
            for (int j = 0; j < m_tableDescriptions.length(); j++)
                m_lastTableValues[j] = m_apuDriver.get_table_values(m_ry)[j];
            for (int j = 0; j < m_tableDescriptions.length(); j++)
                m_tableStatisticData[i][j] = m_apuDriver.get_table_values(m_ry)[j];
        }

        for (int i = 0; i < m_tableDescriptions.length(); i++)
        {
            m_tableStatisticMin[i] = m_tableStatisticData[0][i];
            m_tableStatisticMax[i] = m_tableStatisticData[0][i];
            m_tableStatisticMid[i] = m_tableStatisticData[0][i];
            for (int j = 1; j < STATISTIC_MEASURES_COUNT; j++)
            {
                if (m_tableStatisticMin[i] > m_tableStatisticData[j][i])
                    m_tableStatisticMin[i] = m_tableStatisticData[j][i];
                if (m_tableStatisticMax[i] < m_tableStatisticData[j][i])
                    m_tableStatisticMax[i] = m_tableStatisticData[j][i];
                m_tableStatisticMid[i] += m_tableStatisticData[j][i];
            }
            m_tableStatisticMid[i] /= STATISTIC_MEASURES_COUNT;
        }
    }
#endif

    QStringList resultDescriptions;
    resultDescriptions << "OK "<< "Family unsupported" << "Smu timeout" << "Smu unsupported" << "Smu rejected" << "Error memory access";

    std::function<int(ryzen_access,int)> currentFunction = m_functions[index];
    result = currentFunction(m_ry, value);
    QString resultDescription = (result <= 0) && (result > (resultDescriptions.length() * (-1))) ? resultDescriptions[result * (-1)] : QString("UNKNOWN (%1)").arg(result);

#ifdef  DEBUG_MODE
    if ((index == 0) && (ui->enableExtendedInfo->isChecked()))
    {
        QThread::msleep(1000);
        int errorcode = m_apuDriver.refresh_table(m_ry);
        if (errorcode)
        {
            QMessageBox::warning(this, QString("Table reflesh error after execution"), QString("Table reflesh error after execution %1").arg(errorcode));
            return;
        }

        QStringList info;
        for (int i = 0; i < m_tableDescriptions.length(); i++)
        {
#if 0
            if (((m_lastTableValues[i] - m_apuDriver.get_table_values(m_ry)[i]) > 0.001) || ((m_lastTableValues[i] - m_apuDriver.get_table_values(m_ry)[i]) < -0.001))
                info << QString("#%1. %2: %3 -> %4 (diff %5)").arg(i).arg(m_tableDescriptions[i]).
                        arg(m_lastTableValues[i]).arg(m_apuDriver.get_table_values(m_ry)[i]).arg(m_lastTableValues[i] - m_apuDriver.get_table_values(m_ry)[i]);
#else
            if ((m_apuDriver.get_table_values(m_ry)[i] > (m_tableStatisticMax[i] + MAX_AVALUABLE_DIFF_MULT * m_tableStatisticMid[i])) ||
                    (m_apuDriver.get_table_values(m_ry)[i] < (m_tableStatisticMin[i]) -  MAX_AVALUABLE_DIFF_MULT * m_tableStatisticMid[i]))
                info << QString("#%1. %2: %3 - %4 - %5 ----> %6").arg(i).arg(m_tableDescriptions[i]).
                        arg(m_tableStatisticMin[i]).arg(m_tableStatisticMid[i]).arg(m_tableStatisticMax[i]).arg(m_apuDriver.get_table_values(m_ry)[i]);
#endif
        }

        ui->plainTextEdit->setPlainText(info.join("\n"));
    }
#endif

    if (result == 0)
        QMessageBox::information(this, QString("Execution finished"), QString("Function %1 finished, result %2").arg(m_functionDescriptions[index]).arg(resultDescription));
    else
        QMessageBox::warning(this, QString("Execution finished"), QString("Function %1 finished, result %2").arg(m_functionDescriptions[index]).arg(resultDescription));

    if (ui->enableRefleshTable->isChecked())
        m_periodicTimer.start();
}
