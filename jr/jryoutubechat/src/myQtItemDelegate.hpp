//---------------------------------------------------------------------------
// see https://forum.qt.io/topic/65588/wordwrap-true-problem-in-listview/9
// see also https://stackoverflow.com/questions/64198197/how-to-prevent-too-aggressive-text-elide-in-qtableview

class MyQtWordWrappedListItemDelegate : public QStyledItemDelegate {
public:
	MyQtWordWrappedListItemDelegate(QObject* parent)
		: QStyledItemDelegate(parent) {}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {

		const QAbstractItemModel* model = index.model();
		QString Text = model->data(index, Qt::DisplayRole).toString();

		// kludge
		if (Text.length()>0 && Text[0]==']') {
			return QStyledItemDelegate::sizeHint(option, index);
		}

		QFontMetrics fm(option.font);

		QRect itemRect = QRect(option.rect);

		// idea is we force wordwrap kludge to occur in a box slightly NARROWER, in order to calculate height better; could be due to margin?
		// these values MAY be based on font and should be possibly font-size based?
		int bbExtraWidth = -20;
		int bbExtraHeight = 2000;

		// try to make it know it needs taller?
		itemRect.setWidth(itemRect.width() + bbExtraWidth);
		itemRect.setHeight(itemRect.height() + bbExtraHeight);

		//QRect neededsize = fm.boundingRect(itemRect, Qt::TextWordWrap, Text);
		QRect neededsize = fm.boundingRect(itemRect, Qt::TextWordWrap | Qt::AlignVCenter , Text);

		// and now go back to original width, but we need to bump out the height a bit for reasons unknown; could be due to margin?
		int extraWidth = 0;
		int extraHeight = 8+4;
		neededsize.setWidth(option.rect.width() + extraWidth);
		neededsize.setHeight(neededsize.height() + extraHeight);

		// 0 extra width and 0 extra height actually work reasonably well when we dont do msgList->setSpacing(4) during construction

		//return QSize(itemRect.width() + extraWidth, itemRect.height() + extraHeight);
		return QSize(neededsize.width(), neededsize.height());
	}
};

//---------------------------------------------------------------------------


