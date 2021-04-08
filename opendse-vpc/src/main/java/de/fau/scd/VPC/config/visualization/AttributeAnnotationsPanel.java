// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
 *
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 *
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.config.visualization;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.lang.reflect.InvocationTargetException;
import java.util.Map.Entry;
import java.util.Vector;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.*;

import org.opt4j.core.config.Property;

import de.fau.scd.VPC.config.properties.AttributeAnnotation;
import de.fau.scd.VPC.config.properties.AttributeAnnotations;

@SuppressWarnings("serial")
public class AttributeAnnotationsPanel
    extends
        JScrollPane
    implements
        ActionListener
      , TableModelListener
{

    public AttributeAnnotationsPanel(Property property) {
        this.property = property;
        this.attributeAnnotations = (AttributeAnnotations) property.getValue();

        tableModel = new DefaultTableModel(
            new Object[]{"Spec. element", "attr. name", "attr. type", "attr. value"}, 0);
        for (AttributeAnnotation an : attributeAnnotations) {
            tableModel.addRow(new Object[]{
                an.getElemRegex()
              , an.getAttrName()
              , an.getAttrType()
              , an.getAttrValue().toString() });
        }
        tableModel.addTableModelListener(this);
        table = new JTable(tableModel);

        {
            TableColumn attrTypeColumn = table.getColumnModel().getColumn(2);
            // Set up the editor for the attr. type column.
            JComboBox<AttributeAnnotation.AttrType> comboBox = new JComboBox<AttributeAnnotation.AttrType>();
            for (AttributeAnnotation.AttrType attrType : AttributeAnnotation.AttrType.values())
                comboBox.addItem(attrType);
            attrTypeColumn.setCellEditor(new DefaultCellEditor(comboBox));
//          // Set up tool tips for the attr. type column.
//          DefaultTableCellRenderer renderer =
//                  new DefaultTableCellRenderer();
//          renderer.setToolTipText("Click for combo box");
//          attrTypeColumn.setCellRenderer(renderer);
        }
        this.setViewportView(table);

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd1 = new JMenuItem("New attr. annotation");
            menuItemAdd1.addActionListener(this);
            popupMenu.add(menuItemAdd1);
            menuItemRemove1 = new JMenuItem("Remove selected annotation");
            menuItemRemove1.addActionListener(this);
            popupMenu.add(menuItemRemove1);
            // Set the popup menu for the table
            table.setComponentPopupMenu(popupMenu);
        }

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd2 = new JMenuItem("New attr. annotation");
            menuItemAdd2.addActionListener(this);
            popupMenu.add(menuItemAdd2);
            menuItemRemove2 = new JMenuItem("Remove selected annotation");
            menuItemRemove2.addActionListener(this);
            popupMenu.add(menuItemRemove2);
            // Set the popup menu for the table
            this.setComponentPopupMenu(popupMenu);
        }

//      System.err.println(table.getMinimumSize());
//      System.err.println(table.getPreferredSize());
//      System.err.println(scrollPane.getMinimumSize());
//      System.err.println(scrollPane.getPreferredSize());
        this.setMinimumSize(new Dimension(-1, 22*4));
        this.setPreferredSize(this.getMinimumSize());

    }

    private final Property property;
    private final AttributeAnnotations attributeAnnotations;
    private final DefaultTableModel tableModel;
    private final JTable table;
    private final JMenuItem menuItemAdd1, menuItemAdd2;
    private final JMenuItem menuItemRemove1, menuItemRemove2;

    @Override
    public void actionPerformed(ActionEvent event) {
        JMenuItem menu = (JMenuItem) event.getSource();
        if (menu == menuItemAdd1 || menu == menuItemAdd2) {
            addNewRow();
        } else if (menu == menuItemRemove1 || menu == menuItemRemove2) {
            removeSelectedRows();
        }
    }

    private void addNewRow() {
        tableModel.addRow(new String[0]);
    }

    private void removeSelectedRows() {
        int[] selectedRows = table.getSelectedRows();

        for (int i = selectedRows.length-1; i >= 0; --i) {
            tableModel.removeRow(selectedRows[i]);
        }
    }

    @Override
    public void tableChanged(TableModelEvent e) {
//      System.err.println(e);
        attributeAnnotations.clear();
        @SuppressWarnings("unchecked")
        Vector<Vector<Object>> rows = tableModel.getDataVector();
//      System.err.println(rows);
        for (Vector<Object> row : rows) {
            final String elemRegex = (String) row.get(0);
            final String attrName  = (String) row.get(1);
            final AttributeAnnotation.AttrType attrType = (AttributeAnnotation.AttrType) row.get(2);
            final String attrValue = (String) row.get(3);
            if (elemRegex != null && attrName != null && attrType != null) {
                attributeAnnotations.add(new AttributeAnnotation(
                    elemRegex, attrName, attrType
                  , attrValue != null ? attrValue : ""));
            }
        }
        try {
            property.setValue(attributeAnnotations);
        } catch (InvocationTargetException e1) {
            e1.printStackTrace();
        }
    }

}
