#include "connection_panel.h"
#include "ip_address_input.h"
#include "port_input.h"
#include "connect_toggle.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>

ConnectionPanel::ConnectionPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    // Убрали WA_TransparentForMouseEvents, чтобы элементы управления могли получать события
    
    // Поле ввода IP
    m_ipInput = new IpAddressInput(this);
    
    // Поле ввода порта
    m_portInput = new PortInput(this);
    
    // Тумблер подключения
    m_connectToggle = new ConnectToggle(this);
    connect(m_connectToggle, &ConnectToggle::toggled, this, [this](bool connected) {
        // Блокируем/разблокируем поля ввода
        bool enabled = !connected;
        m_ipInput->setEnabledBool(enabled);
        m_portInput->setEnabledBool(enabled);
        
        sendConnectCommand(connected);
    });
    
    updateControlsPosition();
}

void ConnectionPanel::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible) {
        m_ipInput->show();
        m_portInput->show();
        m_connectToggle->show();
    } else {
        m_ipInput->hide();
        m_portInput->hide();
        m_connectToggle->hide();
    }
    update();
}

void ConnectionPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(30, 30, 30, 40));
    
    // Рамка
    painter.setPen(QPen(QColor(255, 200, 100, 200), 2));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Заголовок
    painter.setPen(QColor(255, 200, 100));
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(10);
    painter.setFont(font);
    painter.drawText(rect().adjusted(5, 5, -5, 22), Qt::AlignHCenter | Qt::AlignTop, "CONNECTION");
}

void ConnectionPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateControlsPosition();
}

void ConnectionPanel::updateControlsPosition()
{
    if (!m_ipInput || !m_portInput || !m_connectToggle) {
        return;
    }
    
    int y = 30;  // Отступ от заголовка
    int leftMargin = 10;
    int spacing = 5;
    
    // Центрируем все элементы по горизонтали
    int totalWidth = m_ipInput->width() + spacing + m_portInput->width() + spacing + m_connectToggle->width();
    int startX = (width() - totalWidth) / 2;
    
    int x = startX;
    m_ipInput->move(x, y);
    x += m_ipInput->width() + spacing;
    
    m_portInput->move(x, y);
    x += m_portInput->width() + spacing;
    
    m_connectToggle->move(x, y);
}

// Методы для отправки команд подвесу

void ConnectionPanel::sendConnectCommand(bool connected)
{
    // TODO: Здесь будет логика отправки команды на подвес
    // Например: gimbal->connect(m_ipInput->ip(), m_portInput->port());
    
    if (connected) {
        qDebug() << "[ConnectionPanel] sendConnectCommand: CONNECTED";
        qDebug() << "  IP:" << m_ipInput->ip();
        qDebug() << "  Port:" << m_portInput->port();
    } else {
        qDebug() << "[ConnectionPanel] sendConnectCommand: DISCONNECTED";
    }
    
    emit connectToggled(connected);
}

void ConnectionPanel::mousePressEvent(QMouseEvent* event)
{
    // Блокируем распространение события к родительским виджетам
    event->accept();
}

void ConnectionPanel::mouseReleaseEvent(QMouseEvent* event)
{
    // Блокируем распространение события к родительским виджетам
    event->accept();
}
