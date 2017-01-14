/*  XmlLabelCreator.cpp
 *
 *  Copyright (C) 2014  Jim Evins <evins@snaught.com>
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

#include "XmlLabelCreator.h"


#include <QByteArray>
#include <QFile>
#include <QTextBlock>
#include <QTextDocument>
#include <QtDebug>

#include "EnumUtil.h"
#include "LabelModel.h"
#include "LabelModelObject.h"
//#include "LabelModelBarcodeObject.h"
#include "LabelModelBoxObject.h"
#include "LabelModelEllipseObject.h"
#include "LabelModelLineObject.h"
#include "LabelModelImageObject.h"
#include "LabelModelTextObject.h"
#include "XmlTemplateCreator.h"
#include "XmlUtil.h"

#include "Merge/None.h"


void
XmlLabelCreator::writeFile( const LabelModel* label, const QString& fileName )
{
	QDomDocument doc;

	createDoc( doc, label );
	QByteArray buffer = doc.toByteArray( 2 );

	QFile file( fileName );

	if ( !file.open( QFile::WriteOnly | QFile::Text) )
	{
		qWarning() << "Error: Cannot write file " << fileName
			   << ": " << file.errorString();
	}

	file.write( buffer.data(), buffer.size() );
}


void
XmlLabelCreator::writeBuffer( const LabelModel* label, QByteArray& buffer )
{
	QDomDocument doc;

	createDoc( doc, label );
	buffer = doc.toByteArray( 2 );
}


void
XmlLabelCreator::serializeObjects( const QList<LabelModelObject*>& objects,
				   QByteArray&                     buffer )
{
	QDomDocument doc;

	QDomNode xmlNode( doc.createProcessingInstruction( "xml", "version=\"1.0\"" ) );
	doc.appendChild( xmlNode );

	QDomElement root = doc.createElement( "Glabels-objects" );
	doc.appendChild( root );

	addObjectsToNode( root, objects );

	buffer = doc.toByteArray( 2 );
}


void
XmlLabelCreator::createDoc( QDomDocument& doc, const LabelModel* label )
{
	QDomNode xmlNode( doc.createProcessingInstruction( "xml", "version=\"1.0\"" ) );
	doc.appendChild( xmlNode );

	QDomElement root = doc.createElement( "Glabels-document" );
	doc.appendChild( root );
	glabels::XmlUtil::setStringAttr( root, "version", "4.0" );

	glabels::XmlTemplateCreator().createTemplateNode( root, label->tmplate() );

	createObjectsNode( root, label );

	if ( label->merge() && !dynamic_cast<merge::None*>(label->merge()) )
	{
		createMergeNode( root, label );
	}

	createDataNode( root, label );
}


void
XmlLabelCreator::createObjectsNode( QDomElement &parent, const LabelModel* label )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Objects" );
	parent.appendChild( node );

	glabels::XmlUtil::setStringAttr( node, "id", "0" );
	glabels::XmlUtil::setBoolAttr( node, "rotate", label->rotate() );

	addObjectsToNode( node, label->objectList() );
}


void
XmlLabelCreator::addObjectsToNode( QDomElement &parent, const QList<LabelModelObject*>& objects )
{
	foreach ( LabelModelObject* object, objects )
	{
		if ( LabelModelBoxObject* boxObject = dynamic_cast<LabelModelBoxObject*>(object) )
		{
			createObjectBoxNode( parent, boxObject );
		}
		else if ( LabelModelEllipseObject* ellipseObject = dynamic_cast<LabelModelEllipseObject*>(object) )
		{
			createObjectEllipseNode( parent, ellipseObject );
		}
		else if ( LabelModelLineObject* lineObject = dynamic_cast<LabelModelLineObject*>(object) )
		{
			createObjectLineNode( parent, lineObject );
		}
		else if ( LabelModelImageObject* imageObject = dynamic_cast<LabelModelImageObject*>(object) )
		{
			createObjectImageNode( parent, imageObject );
		}
		else if ( LabelModelTextObject* textObject = dynamic_cast<LabelModelTextObject*>(object) )
		{
			createObjectTextNode( parent, textObject );
		}
		// TODO: other object types
		else
		{
			Q_ASSERT_X( false, "XmlLabelCreator::addObjectsToNode", "Invalid object type." );
		}
	}
}


void
XmlLabelCreator::createObjectBoxNode( QDomElement &parent, const LabelModelBoxObject* object )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Object-box" );
	parent.appendChild( node );

	/* position attrs */
	glabels::XmlUtil::setLengthAttr( node, "x", object->x0() );
	glabels::XmlUtil::setLengthAttr( node, "y", object->y0() );

	/* size attrs */
	glabels::XmlUtil::setLengthAttr( node, "w", object->w() );
	glabels::XmlUtil::setLengthAttr( node, "h", object->h() );

	/* line attrs */
	glabels::XmlUtil::setLengthAttr( node, "line_width", object->lineWidth() );
	if ( object->lineColorNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "line_color_field", object->lineColorNode().key() );
	}
	else
	{
		glabels::XmlUtil::setUIntAttr( node, "line_color", object->lineColorNode().rgba() );
	}

	/* fill attrs */
	if ( object->fillColorNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "fill_color_field", object->fillColorNode().key() );
	}
	else
	{
		glabels::XmlUtil::setUIntAttr( node, "fill_color", object->fillColorNode().rgba() );
	}

	/* affine attrs */
	createAffineAttrs( node, object );

	/* shadow attrs */
	createShadowAttrs( node, object );
}


void
XmlLabelCreator::createObjectEllipseNode( QDomElement &parent, const LabelModelEllipseObject* object )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Object-ellipse" );
	parent.appendChild( node );

	/* position attrs */
	glabels::XmlUtil::setLengthAttr( node, "x", object->x0() );
	glabels::XmlUtil::setLengthAttr( node, "y", object->y0() );

	/* size attrs */
	glabels::XmlUtil::setLengthAttr( node, "w", object->w() );
	glabels::XmlUtil::setLengthAttr( node, "h", object->h() );

	/* line attrs */
	glabels::XmlUtil::setLengthAttr( node, "line_width", object->lineWidth() );
	if ( object->lineColorNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "line_color_field", object->lineColorNode().key() );
	}
	else
	{
		glabels::XmlUtil::setUIntAttr( node, "line_color", object->lineColorNode().rgba() );
	}

	/* fill attrs */
	if ( object->fillColorNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "fill_color_field", object->fillColorNode().key() );
	}
	else
	{
		glabels::XmlUtil::setUIntAttr( node, "fill_color", object->fillColorNode().rgba() );
	}

	/* affine attrs */
	createAffineAttrs( node, object );

	/* shadow attrs */
	createShadowAttrs( node, object );
}


void
XmlLabelCreator::createObjectLineNode( QDomElement &parent, const LabelModelLineObject* object )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Object-line" );
	parent.appendChild( node );

	/* position attrs */
	glabels::XmlUtil::setLengthAttr( node, "x", object->x0() );
	glabels::XmlUtil::setLengthAttr( node, "y", object->y0() );

	/* size attrs */
	glabels::XmlUtil::setLengthAttr( node, "dx", object->w() );
	glabels::XmlUtil::setLengthAttr( node, "dy", object->h() );

	/* line attrs */
	glabels::XmlUtil::setLengthAttr( node, "line_width", object->lineWidth() );
	if ( object->lineColorNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "line_color_field", object->lineColorNode().key() );
	}
	else
	{
		glabels::XmlUtil::setUIntAttr( node, "line_color", object->lineColorNode().rgba() );
	}

	/* affine attrs */
	createAffineAttrs( node, object );

	/* shadow attrs */
	createShadowAttrs( node, object );
}


void
XmlLabelCreator::createObjectImageNode( QDomElement &parent, const LabelModelImageObject* object )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Object-image" );
	parent.appendChild( node );

	/* position attrs */
	glabels::XmlUtil::setLengthAttr( node, "x", object->x0() );
	glabels::XmlUtil::setLengthAttr( node, "y", object->y0() );

	/* size attrs */
	glabels::XmlUtil::setLengthAttr( node, "w", object->w() );
	glabels::XmlUtil::setLengthAttr( node, "h", object->h() );

	/* file attrs */
	glabels::XmlUtil::setLengthAttr( node, "line_width", object->lineWidth() );
	if ( object->filenameNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "src_field", object->filenameNode().data() );
	}
	else
	{
		glabels::XmlUtil::setStringAttr( node, "src", object->filenameNode().data() );
	}

	/* affine attrs */
	createAffineAttrs( node, object );

	/* shadow attrs */
	createShadowAttrs( node, object );
}


void
XmlLabelCreator::createObjectBarcodeNode( QDomElement &parent, const LabelModelBarcodeObject* object )
{
	// TODO
}


void
XmlLabelCreator::createObjectTextNode( QDomElement &parent, const LabelModelTextObject* object )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Object-text" );
	parent.appendChild( node );

	/* position attrs */
	glabels::XmlUtil::setLengthAttr( node, "x", object->x0() );
	glabels::XmlUtil::setLengthAttr( node, "y", object->y0() );

	/* size attrs */
	glabels::XmlUtil::setLengthAttr( node, "w", object->w() );
	glabels::XmlUtil::setLengthAttr( node, "h", object->h() );

	/* color attr */
	if ( object->textColorNode().isField() )
	{
		glabels::XmlUtil::setStringAttr( node, "color_field", object->textColorNode().key() );
	}
	else
	{
		glabels::XmlUtil::setUIntAttr( node, "color", object->textColorNode().rgba() );
	}

	/* font attrs */
	glabels::XmlUtil::setStringAttr( node, "font_family", object->fontFamily() );
	glabels::XmlUtil::setDoubleAttr( node, "font_size", object->fontSize() );
	glabels::XmlUtil::setStringAttr( node, "font_weight", EnumUtil::weightToString( object->fontWeight() ) );
	glabels::XmlUtil::setBoolAttr( node, "font_italic", object->fontItalicFlag() );
	glabels::XmlUtil::setBoolAttr( node, "font_underline", object->fontUnderlineFlag() );

	/* text attrs */
	glabels::XmlUtil::setDoubleAttr( node, "line_spacing", object->textLineSpacing() );
	glabels::XmlUtil::setStringAttr( node, "align", EnumUtil::hAlignToString( object->textHAlign() ) );
	glabels::XmlUtil::setStringAttr( node, "valign", EnumUtil::vAlignToString( object->textVAlign() ) );

	/* affine attrs */
	createAffineAttrs( node, object );

	/* shadow attrs */
	createShadowAttrs( node, object );

	/* serialize text contents */
	QTextDocument document( object->text() );
	int nBlocks = document.blockCount();
	for ( int iBlock = 0; iBlock < nBlocks; iBlock++ )
	{
		createPNode( node, document.findBlockByNumber(iBlock).text() );
	}
}


void
XmlLabelCreator::createPNode( QDomElement &parent, const QString& blockText )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "p" );
	parent.appendChild( node );

	node.appendChild( doc.createTextNode( blockText ) );
}


void
XmlLabelCreator::createAffineAttrs( QDomElement &node, const LabelModelObject* object )
{
	QMatrix a = object->matrix();
	
	glabels::XmlUtil::setDoubleAttr( node, "a0", a.m11() );
	glabels::XmlUtil::setDoubleAttr( node, "a1", a.m12() );
	glabels::XmlUtil::setDoubleAttr( node, "a2", a.m21() );
	glabels::XmlUtil::setDoubleAttr( node, "a3", a.m22() );
	glabels::XmlUtil::setDoubleAttr( node, "a4", a.dx() );
	glabels::XmlUtil::setDoubleAttr( node, "a5", a.dy() );
}


void
XmlLabelCreator::createShadowAttrs( QDomElement &node, const LabelModelObject* object )
{
	if ( object->shadow() )
	{
		glabels::XmlUtil::setBoolAttr( node, "shadow", object->shadow() );

		glabels::XmlUtil::setLengthAttr( node, "shadow_x", object->shadowX() );
		glabels::XmlUtil::setLengthAttr( node, "shadow_y", object->shadowY() );

		if ( object->fillColorNode().isField() )
		{
			glabels::XmlUtil::setStringAttr( node, "shadow_color_field", object->shadowColorNode().key() );
		}
		else
		{
			glabels::XmlUtil::setUIntAttr( node, "shadow_color", object->shadowColorNode().rgba() );
		}

		glabels::XmlUtil::setDoubleAttr( node, "shadow_opacity", object->shadowOpacity() );
	}
}


void
XmlLabelCreator::createMergeNode( QDomElement &parent, const LabelModel* label )
{
	QDomDocument doc = parent.ownerDocument();
	QDomElement node = doc.createElement( "Merge" );
	parent.appendChild( node );

	glabels::XmlUtil::setStringAttr( node, "type", label->merge()->id() );
	glabels::XmlUtil::setStringAttr( node, "src", label->merge()->source() );
}


void
XmlLabelCreator::createDataNode( QDomElement &parent, const LabelModel* label )
{
	// TODO
}


void
XmlLabelCreator::createPixdataNode( QDomElement &parent, const LabelModel* label, const QString& name )
{
	// TODO
}


void
XmlLabelCreator::createSvgFileNode( QDomElement &parent, const LabelModel* label, const QString& name )
{
	// TODO
}

