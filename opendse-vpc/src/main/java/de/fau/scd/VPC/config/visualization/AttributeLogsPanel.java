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

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.lang.reflect.InvocationTargetException;
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

import de.fau.scd.VPC.config.properties.AttributeLog;
import de.fau.scd.VPC.config.properties.AttributeLogs;

@SuppressWarnings("serial")
public class AttributeLogsPanel
    extends
        JScrollPane
    implements
        ActionListener
      , TableModelListener
{

    public AttributeLogsPanel(Property property) {
        this.property = property;
        this.attributeLogs = (AttributeLogs) property.getValue();

        tableModel = new DefaultTableModel(
            new Object[]{"type", "id. regex", "attr. name"}, 0);
        for (AttributeLog al : attributeLogs) {
            tableModel.addRow(new Object[]{
                al.getNodeType()
              , al.getElemRegex().pattern()
              , al.getAttrName() });
        }
        tableModel.addTableModelListener(this);
        table = new JTable(tableModel);

        {
            TableColumn nodeTypeColumn = table.getColumnModel().getColumn(0);
            // Set up the editor for the node type column.
            JComboBox<AttributeLog.NodeType> comboBox = new JComboBox<AttributeLog.NodeType>();
            for (AttributeLog.NodeType nodeType : AttributeLog.NodeType.values())
                comboBox.addItem(nodeType);
            nodeTypeColumn.setCellEditor(new DefaultCellEditor(comboBox));
//          // Set up tool tips for the node type column.
//          DefaultTableCellRenderer renderer =
//                  new DefaultTableCellRenderer();
//          renderer.setToolTipText("Click for combo box");
//          attrTypeColumn.setCellRenderer(renderer);
        }
        this.setViewportView(table);

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd1 = new JMenuItem("New attr. log");
            menuItemAdd1.addActionListener(this);
            popupMenu.add(menuItemAdd1);
            menuItemRemove1 = new JMenuItem("Remove selected attr. log");
            menuItemRemove1.addActionListener(this);
            popupMenu.add(menuItemRemove1);
            // Set the popup menu for the table
            table.setComponentPopupMenu(popupMenu);
        }

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd2 = new JMenuItem("New attr. log");
            menuItemAdd2.addActionListener(this);
            popupMenu.add(menuItemAdd2);
            menuItemRemove2 = new JMenuItem("Remove selected attr. log");
            menuItemRemove2.addActionListener(this);
            popupMenu.add(menuItemRemove2);
            // Set the popup menu for the table
            this.setComponentPopupMenu(popupMenu);
        }

        this.setMinimumSize(new Dimension(-1, 22*4));
        this.setPreferredSize(this.getMinimumSize());
    }

    private final Property property;
    private final AttributeLogs attributeLogs;
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
        attributeLogs.clear();
        @SuppressWarnings("unchecked")
        Vector<Vector<Object>> rows = tableModel.getDataVector();
//      System.err.println(rows);
        for (Vector<Object> row : rows) {
            final AttributeLog.NodeType nodeType = (AttributeLog.NodeType) row.get(0);
            final String elemRegex = (String) row.get(1);
            final String attrName  = (String) row.get(2);
            if (nodeType != null && attrName != null) {
                attributeLogs.add(new AttributeLog(
                    nodeType, elemRegex != null ? elemRegex : "", attrName));
            }
        }
        try {
            property.setValue(attributeLogs);
        } catch (InvocationTargetException e1) {
            e1.printStackTrace();
        }
    }

}
