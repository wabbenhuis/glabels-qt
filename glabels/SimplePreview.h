/*  SimplePreview.h
 *
 *  Copyright (C) 2013-2016  Jim Evins <evins@snaught.com>
 *
 *  This file is part of gLabels-qt.
 *
 *  gLabels-qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels-qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels-qt.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SimplePreview_h
#define SimplePreview_h


#include <QGraphicsView>
#include <QGraphicsScene>
#include <QList>

#include "Template.h"


///
/// Simple Preview Widget
///
class SimplePreview : public QGraphicsView
{
	Q_OBJECT


	/////////////////////////////////
	// Life Cycle
	/////////////////////////////////
public:
	SimplePreview( QWidget *parent = 0 );


	/////////////////////////////////
	// Properties
	/////////////////////////////////
public:
	void setTemplate( const glabels::Template *tmplate );
	void setRotate( bool rotateFlag );


	/////////////////////////////////////
	// Event handlers
	/////////////////////////////////////
protected:
	void resizeEvent( QResizeEvent* event );

		
	/////////////////////////////////
	// Internal Methods
	/////////////////////////////////
private:
	void update();
	void clearScene();
	void drawPaper( const glabels::Distance& pw, const glabels::Distance& ph );
	void drawLabels();
	void drawLabel( const glabels::Distance& x,
	                const glabels::Distance& y,
	                const QPainterPath&      path );
	void drawArrow();


	/////////////////////////////////
	// Private Data
	/////////////////////////////////
private:
	const glabels::Template *mTmplate;
	bool                     mRotateFlag;

	QGraphicsScene          *mScene;

};


#endif // SimplePreview_h
